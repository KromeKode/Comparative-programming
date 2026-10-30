#!/usr/bin/env python3
"""Build and test exercise solutions in all nine languages.

Every exercise is a program that reads stdin and writes stdout. Each test
case is a pair of files, NAME.in and NAME.out, in the exercise's tests/
folder. This script runs your program on every .in file and compares what it
prints with the matching .out file. When one language's correct output is
different, it gets its own file, such as NAME.python.out.

Examples:
    python3 tools/check.py --doctor            # which toolchains are installed?
    python3 tools/check.py 01                  # check your chapter 1 exercises
    python3 tools/check.py 01 --lang rust,go   # only some languages
    python3 tools/check.py 01 --solutions      # check the reference solutions
    python3 tools/check.py all --solutions     # what CI runs
"""

from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
import tempfile
from dataclasses import dataclass, field
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
CHAPTERS = ROOT / "chapters"
TIMEOUT_SECONDS = 20


@dataclass
class Language:
    name: str
    source: str                      # file name inside the language folder
    tools: list[str]                 # programs that must be on PATH
    build: list[list[str]] = field(default_factory=list)  # commands run once
    run: list[str] = field(default_factory=list)          # command per test
    version: list[str] = field(default_factory=list)      # for --doctor


# {src} = the source file, {out} = a temporary build folder.
LANGUAGES: dict[str, Language] = {
    "c": Language(
        "C", "main.c", ["cc"],
        build=[["cc", "-std=c17", "-Wall", "-Wextra", "-Werror", "-O2",
                "-o", "{out}/main", "{src}", "-lm"]],
        run=["{out}/main"],
        version=["cc", "--version"],
    ),
    "cpp": Language(
        "C++", "main.cpp", ["c++"],
        build=[["c++", "-std=c++20", "-Wall", "-Wextra", "-Werror", "-O2",
                "-o", "{out}/main", "{src}"]],
        run=["{out}/main"],
        version=["c++", "--version"],
    ),
    "rust": Language(
        "Rust", "main.rs", ["rustc"],
        build=[["rustc", "--edition", "2024", "-O", "-D", "warnings",
                "-o", "{out}/main", "{src}"]],
        run=["{out}/main"],
        version=["rustc", "--version"],
    ),
    "go": Language(
        "Go", "main.go", ["go"],
        build=[["go", "vet", "{src}"], ["go", "build", "-o", "{out}/main", "{src}"]],
        run=["{out}/main"],
        version=["go", "version"],
    ),
    "python": Language(
        "Python", "main.py", ["python3"],
        build=[["python3", "-m", "py_compile", "{src}"]],
        run=["python3", "{src}"],
        version=["python3", "--version"],
    ),
    "javascript": Language(
        "JavaScript", "main.js", ["node"],
        build=[["node", "--check", "{src}"]],
        run=["node", "{src}"],
        version=["node", "--version"],
    ),
    "typescript": Language(
        "TypeScript", "main.ts", ["node"],
        # The type check needs `npm install` (it uses node_modules/.bin/tsc).
        build=[[str(ROOT / "node_modules" / ".bin" / "tsc"), "--noEmit", "--strict",
                "--target", "esnext", "--module", "nodenext",
                "--erasableSyntaxOnly", "--types", "node", "{src}"]],
        run=["node", "{src}"],   # Node 22.18+ strips the types and runs the file
        version=[str(ROOT / "node_modules" / ".bin" / "tsc"), "--version"],
    ),
    "dart": Language(
        "Dart", "main.dart", ["dart"],
        build=[["dart", "analyze", "--fatal-infos", "{src}"]],
        run=["dart", "run", "{src}"],
        version=["dart", "--version"],
    ),
    "kotlin": Language(
        "Kotlin", "main.kt", ["kotlinc", "java"],
        build=[["kotlinc", "{src}", "-include-runtime", "-Werror", "-d", "{out}/main.jar"]],
        # The JVM encodes stdout using the system locale unless told otherwise, so
        # on a machine without a UTF-8 locale "Zoë" would print as "Zo?".
        run=["java", "-Dstdout.encoding=UTF-8", "-Dstderr.encoding=UTF-8",
             "-jar", "{out}/main.jar"],
        version=["kotlinc", "-version"],
    ),
}

GREEN, RED, YELLOW, DIM, RESET = (
    ("\033[32m", "\033[31m", "\033[33m", "\033[2m", "\033[0m")
    if sys.stdout.isatty() and os.environ.get("NO_COLOR") is None
    else ("", "", "", "", "")
)


def missing_tools(lang: Language) -> list[str]:
    missing = [t for t in lang.tools if shutil.which(t) is None]
    if lang.name == "TypeScript" and not (ROOT / "node_modules" / ".bin" / "tsc").exists():
        missing.append("tsc (run `npm install` in the repo root)")
    return missing


def message_lines(text: str) -> list[str]:
    """Split tool output into lines, dropping the JVM's "Picked up JAVA_TOOL_OPTIONS" notes."""
    return [line for line in text.strip().splitlines() if not line.startswith("Picked up ")]


def fill(args: list[str], src: Path, out: Path) -> list[str]:
    return [a.replace("{src}", str(src)).replace("{out}", str(out)) for a in args]


def normalize(text: str) -> list[str]:
    """Compare output line by line, ignoring trailing spaces and blank lines at the end."""
    lines = [line.rstrip() for line in text.replace("\r\n", "\n").split("\n")]
    while lines and lines[-1] == "":
        lines.pop()
    return lines


def first_difference(got: list[str], want: list[str]) -> str:
    for i in range(max(len(got), len(want))):
        g = got[i] if i < len(got) else "<no line>"
        w = want[i] if i < len(want) else "<no line>"
        if g != w:
            return f"line {i + 1}: expected {w!r}, got {g!r}"
    return "outputs differ"


def check_one(key: str, lang: Language, src: Path, tests: list[Path]) -> tuple[str, str]:
    """Return (status, detail) for one language's program on all test cases."""
    with tempfile.TemporaryDirectory(prefix="check-") as tmp:
        out = Path(tmp)
        for cmd in lang.build:
            proc = subprocess.run(fill(cmd, src, out), capture_output=True, text=True,
                                  cwd=src.parent, timeout=120)
            if proc.returncode != 0:
                message = message_lines(proc.stderr) or message_lines(proc.stdout)
                return "FAIL", "build: " + (message[0] if message else "failed")
        for case in tests:
            # A language whose correct output differs (say, Python printing True
            # where others print true) has its own NAME.<language>.out file.
            expected = case.with_suffix(f".{key}.out")
            if not expected.exists():
                expected = case.with_suffix(".out")
            if not expected.exists():
                return "FAIL", f"{case.stem}: no {case.stem}.out or {case.stem}.{key}.out file"
            want = normalize(expected.read_text(encoding="utf-8"))
            try:
                proc = subprocess.run(fill(lang.run, src, out), input=case.read_bytes(),
                                      capture_output=True, cwd=src.parent,
                                      timeout=TIMEOUT_SECONDS)
            except subprocess.TimeoutExpired:
                return "FAIL", f"{case.stem}: timed out after {TIMEOUT_SECONDS}s"
            if proc.returncode != 0:
                err = message_lines(proc.stderr.decode("utf-8", "replace"))
                return "FAIL", f"{case.stem}: exit code {proc.returncode}" + (
                    f" ({err[-1]})" if err else "")
            got = normalize(proc.stdout.decode("utf-8", "replace"))
            if got != want:
                return "FAIL", f"{case.stem}: {first_difference(got, want)}"
    return "PASS", f"{len(tests)} test{'s' if len(tests) != 1 else ''}"


def find_chapters(selector: str) -> list[Path]:
    chapters = sorted(p for p in CHAPTERS.iterdir() if p.is_dir())
    if selector == "all":
        return chapters
    wanted = selector.zfill(2)
    return [p for p in chapters if p.name.startswith(wanted + "-")]


def doctor() -> int:
    print("Toolchains:")
    for key, lang in LANGUAGES.items():
        missing = missing_tools(lang)
        if missing:
            print(f"  {RED}missing{RESET}  {lang.name:<11} needs {', '.join(missing)}")
            continue
        proc = subprocess.run(lang.version, capture_output=True, text=True)
        first = message_lines(proc.stdout) or message_lines(proc.stderr)
        print(f"  {GREEN}ok{RESET}       {lang.name:<11} {first[0] if first else ''}")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("chapter", nargs="?", default="all",
                        help="chapter number such as 01, or 'all' (default)")
    parser.add_argument("--solutions", action="store_true",
                        help="check the reference solutions instead of your exercises")
    parser.add_argument("--lang", default="",
                        help="comma-separated languages: " + ",".join(LANGUAGES))
    parser.add_argument("--doctor", action="store_true", help="list installed toolchains")
    args = parser.parse_args()

    if args.doctor:
        return doctor()

    langs = [l.strip() for l in args.lang.split(",") if l.strip()] or list(LANGUAGES)
    unknown = [l for l in langs if l not in LANGUAGES]
    if unknown:
        parser.error(f"unknown language(s): {', '.join(unknown)}")

    chapters = find_chapters(args.chapter)
    if not chapters:
        parser.error(f"no chapter matches {args.chapter!r}")

    # (folder with programs, folder whose <name>/tests holds the test cases)
    groups = [("examples", "examples"), ("solutions", "exercises")] if args.solutions \
        else [("exercises", "exercises")]
    counts = {"PASS": 0, "FAIL": 0, "SKIP": 0}
    for chapter in chapters:
        for folder, tests_folder in groups:
            program_root = chapter / folder
            if not program_root.is_dir():
                continue
            print(f"\n{chapter.name} ({folder})")
            for program in sorted(p for p in program_root.iterdir() if p.is_dir()):
                tests = sorted((chapter / tests_folder / program.name / "tests").glob("*.in"))
                if not tests:
                    continue
                print(f"  {program.name}")
                for key in langs:
                    lang = LANGUAGES[key]
                    src = program / key / lang.source
                    if not src.exists():
                        continue
                    missing = missing_tools(lang)
                    if missing:
                        status, detail = "SKIP", "needs " + ", ".join(missing)
                    else:
                        status, detail = check_one(key, lang, src, tests)
                    counts[status] += 1
                    color = {"PASS": GREEN, "FAIL": RED, "SKIP": YELLOW}[status]
                    print(f"    {color}{status}{RESET}  {lang.name:<11} {DIM}{detail}{RESET}")

    print(f"\n{counts['PASS']} passed, {counts['FAIL']} failed, {counts['SKIP']} skipped")
    return 1 if counts["FAIL"] else 0


if __name__ == "__main__":
    sys.exit(main())
