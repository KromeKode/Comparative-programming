#!/usr/bin/env python3
"""Build and test exercise solutions in all nine languages.

Every exercise is a program that reads stdin and writes stdout. Each test
case is a pair of files, NAME.in and NAME.out, in the exercise's tests/
folder. This script runs your program on every .in file and compares what it
prints with the matching .out file. When one language's correct output is
different, it gets its own file, such as NAME.python.out.

A program can be several files. The C, C++, and Kotlin compilers get every
source file in the language's folder, and the other languages follow imports.
From chapter 18, a folder can also be a project for a build tool, such as a
Cargo.toml or a CMakeLists.txt, and then that tool builds it.

Chapter 17 is about Git, so its programs are shell scripts of Git commands
(main.sh in a git folder). There each NAME.in is a script that sets up a
repository, and tools/git-sandbox.sh runs your script in it.

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
class Project:
    """A language folder that a build tool builds, found by the file it reads."""
    marker: str                      # such as Cargo.toml
    tools: list[str]
    build: list[list[str]]
    run: list[str]


@dataclass
class Language:
    name: str
    source: str                      # file name inside the language folder
    tools: list[str]                 # programs that must be on PATH
    build: list[list[str]] = field(default_factory=list)  # commands run once
    run: list[str] = field(default_factory=list)          # command per test
    version: list[str] = field(default_factory=list)      # for --doctor
    projects: list[Project] = field(default_factory=list)  # chapter 18 on


# Commands run in the language folder. {src} = the source file, {srcs} = every
# source file in the folder and below it (one argument each), {out} = a temporary
# build folder, {tests} = the folder of test cases, {folder} = examples, exercises,
# or solutions.
CMAKE = Project(
    "CMakeLists.txt", ["cmake"],
    build=[["cmake", "-S", ".", "-B", "{out}", "-DCMAKE_BUILD_TYPE=Release"],
           ["cmake", "--build", "{out}"]],
    run=["{out}/main"],
)
NPM_INSTALL = ["npm", "ci", "--offline", "--no-audit", "--no-fund", "--silent"]
TSC = str(ROOT / "node_modules" / ".bin" / "tsc")
# --allowImportingTsExtensions lets one file import another as "./other.ts",
# which is the path Node needs.
TSC_CHECK = [TSC, "--noEmit", "--strict", "--target", "esnext", "--module", "nodenext",
             "--erasableSyntaxOnly", "--allowImportingTsExtensions", "--types", "node",
             "{src}"]
LANGUAGES: dict[str, Language] = {
    "c": Language(
        "C", "main.c", ["cc"],
        build=[["cc", "-std=c17", "-Wall", "-Wextra", "-Werror", "-O2",
                "-o", "{out}/main", "{srcs}", "-lm"]],
        run=["{out}/main"],
        version=["cc", "--version"],
        projects=[CMAKE],
    ),
    "cpp": Language(
        "C++", "main.cpp", ["c++"],
        build=[["c++", "-std=c++20", "-Wall", "-Wextra", "-Werror", "-O2",
                "-o", "{out}/main", "{srcs}"]],
        run=["{out}/main"],
        version=["c++", "--version"],
        projects=[CMAKE],
    ),
    "rust": Language(
        "Rust", "main.rs", ["rustc"],
        build=[["rustc", "--edition", "2024", "-O", "-D", "warnings",
                "-o", "{out}/main", "{src}"]],
        run=["{out}/main"],
        version=["rustc", "--version"],
        projects=[Project(
            "Cargo.toml", ["cargo"],
            build=[["env", "RUSTFLAGS=-D warnings", "cargo", "build", "--release",
                    "--offline", "--quiet", "--target-dir", "{out}"]],
            run=["{out}/release/main"],
        )],
    ),
    "go": Language(
        "Go", "main.go", ["go"],
        build=[["go", "vet", "{srcs}"], ["go", "build", "-o", "{out}/main", "{srcs}"]],
        run=["{out}/main"],
        version=["go", "version"],
        # A module: packages in folders of their own, imported by path.
        projects=[Project(
            "go.mod", ["go"],
            build=[["go", "vet", "./..."], ["go", "build", "-o", "{out}/main", "."]],
            run=["{out}/main"],
        )],
    ),
    "python": Language(
        "Python", "main.py", ["python3"],
        build=[["python3", "-m", "py_compile", "{src}"]],
        run=["python3", "{src}"],
        version=["python3", "--version"],
        # uv installs the dependencies in uv.lock into a virtual environment.
        projects=[Project(
            "pyproject.toml", ["uv"],
            build=[["env", "UV_PROJECT_ENVIRONMENT={out}/venv", "uv", "sync", "--locked",
                    "--quiet"]],
            run=["{out}/venv/bin/python", "main.py"],
        )],
    ),
    "javascript": Language(
        "JavaScript", "main.js", ["node"],
        build=[["node", "--check", "{src}"]],
        run=["node", "{src}"],
        version=["node", "--version"],
        projects=[Project(
            "package.json", ["npm", "node"],
            build=[NPM_INSTALL, ["node", "--check", "main.js"]],
            run=["node", "main.js"],
        )],
    ),
    "typescript": Language(
        "TypeScript", "main.ts", ["node"],
        # The type check needs `npm install` (it uses node_modules/.bin/tsc).
        build=[TSC_CHECK],
        run=["node", "{src}"],   # Node 22.18+ strips the types and runs the file
        version=[TSC, "--version"],
        projects=[Project(
            "package.json", ["npm", "node"],
            build=[NPM_INSTALL, [a.replace("{src}", "main.ts") for a in TSC_CHECK]],
            run=["node", "main.ts"],
        )],
    ),
    "dart": Language(
        "Dart", "main.dart", ["dart"],
        build=[["dart", "analyze", "--fatal-infos", "."]],
        run=["dart", "run", "{src}"],
        version=["dart", "--version"],
        # A package: pub reads pubspec.yaml, and the program is in bin/.
        projects=[Project(
            "pubspec.yaml", ["dart"],
            build=[["dart", "pub", "get", "--offline"], ["dart", "analyze", "--fatal-infos"]],
            run=["dart", "run", "bin/main.dart"],
        )],
    ),
    "kotlin": Language(
        "Kotlin", "main.kt", ["kotlinc", "java"],
        build=[["kotlinc", "{srcs}", "-include-runtime", "-Werror", "-d", "{out}/main.jar"]],
        # The JVM encodes stdout using the system locale unless told otherwise, so
        # on a machine without a UTF-8 locale "Zoë" would print as "Zo?".
        run=["java", "-Dstdout.encoding=UTF-8", "-Dstderr.encoding=UTF-8",
             "-jar", "{out}/main.jar"],
        version=["kotlinc", "-version"],
        # A Gradle build, whose installDist task writes a script that runs the program.
        projects=[Project(
            "settings.gradle.kts", ["gradle", "java"],
            build=[["gradle", "--quiet", "installDist"]],
            run=["build/install/main/bin/main"],
        )],
    ),
    # Chapter 17 only: a bash script of Git commands, run in a throwaway repository.
    "git": Language(
        "Git", "main.sh", ["git", "bash"],
        build=[["bash", "-n", "{src}"]],
        run=["bash", str(ROOT / "tools" / "git-sandbox.sh"), "{folder}", "{src}", "{tests}"],
        version=["git", "--version"],
    ),
}

GREEN, RED, YELLOW, DIM, RESET = (
    ("\033[32m", "\033[31m", "\033[33m", "\033[2m", "\033[0m")
    if sys.stdout.isatty() and os.environ.get("NO_COLOR") is None
    else ("", "", "", "", "")
)


@dataclass
class Plan:
    """How to build and run one language's program: as a single file, or a project."""
    src: Path                        # the source file, or the project's marker file
    tools: list[str]
    build: list[list[str]]
    run: list[str]


def find_plan(lang: Language, folder: Path) -> Plan | None:
    for project in lang.projects:
        if (folder / project.marker).exists():
            return Plan(folder / project.marker, project.tools, project.build, project.run)
    if (folder / lang.source).exists():
        return Plan(folder / lang.source, lang.tools, lang.build, lang.run)
    return None


def missing_tools(lang: Language, tools: list[str]) -> list[str]:
    missing = [t for t in tools if shutil.which(t) is None]
    if lang.name == "TypeScript" and not (ROOT / "node_modules" / ".bin" / "tsc").exists():
        missing.append("tsc (run `npm install` in the repo root)")
    return missing


def message_lines(text: str) -> list[str]:
    """Split tool output into lines, dropping the JVM's "Picked up JAVA_TOOL_OPTIONS" notes."""
    return [line for line in text.strip().splitlines() if not line.startswith("Picked up ")]


def fill(args: list[str], src: Path, out: Path, tests: Path) -> list[str]:
    names = {"{src}": str(src), "{out}": str(out), "{tests}": str(tests),
             "{folder}": src.parent.parent.parent.name}
    filled = []
    for arg in args:
        if arg == "{srcs}":
            filled += [str(path) for path in sorted(src.parent.rglob("*" + src.suffix))]
            continue
        for name, value in names.items():
            arg = arg.replace(name, value)
        filled.append(arg)
    return filled


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


def first_error(lines: list[str]) -> str:
    """The first line that mentions an error, or else the first line."""
    for line in lines:
        if "error" in line.lower():
            return line.strip()
    return lines[0] if lines else "failed"


def check_one(key: str, plan: Plan, tests: list[Path]) -> tuple[str, str]:
    """Return (status, detail) for one language's program on all test cases."""
    src = plan.src
    with tempfile.TemporaryDirectory(prefix="check-") as tmp:
        out = Path(tmp)
        folder = tests[0].parent
        for cmd in plan.build:
            # A build tool's first build can download what it needs, so it gets longer.
            proc = subprocess.run(fill(cmd, src, out, folder), capture_output=True, text=True,
                                  cwd=src.parent, timeout=600)
            if proc.returncode != 0:
                message = message_lines(proc.stderr) or message_lines(proc.stdout)
                return "FAIL", "build: " + first_error(message)
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
                result = subprocess.run(fill(plan.run, src, out, folder),
                                        input=case.read_bytes(), capture_output=True,
                                        cwd=src.parent, timeout=TIMEOUT_SECONDS)
            except subprocess.TimeoutExpired:
                return "FAIL", f"{case.stem}: timed out after {TIMEOUT_SECONDS}s"
            if result.returncode != 0:
                err = message_lines(result.stderr.decode("utf-8", "replace"))
                return "FAIL", f"{case.stem}: exit code {result.returncode}" + (
                    f" ({err[-1]})" if err else "")
            got = normalize(result.stdout.decode("utf-8", "replace"))
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
        missing = missing_tools(lang, lang.tools)
        if missing:
            print(f"  {RED}missing{RESET}  {lang.name:<11} needs {', '.join(missing)}")
            continue
        proc = subprocess.run(lang.version, capture_output=True, text=True)
        first = message_lines(proc.stdout) or message_lines(proc.stderr)
        print(f"  {GREEN}ok{RESET}       {lang.name:<11} {first[0] if first else ''}")
    print("Build tools (chapter 18 on):")
    for tool in ["cmake", "cargo", "uv", "npm", "gradle"]:
        if shutil.which(tool) is None:
            print(f"  {YELLOW}missing{RESET}  {tool}")
        else:
            print(f"  {GREEN}ok{RESET}       {tool}")
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
                    plan = find_plan(lang, program / key)
                    if plan is None:
                        continue
                    missing = missing_tools(lang, plan.tools)
                    if missing:
                        status, detail = "SKIP", "needs " + ", ".join(missing)
                    else:
                        status, detail = check_one(key, plan, tests)
                    counts[status] += 1
                    color = {"PASS": GREEN, "FAIL": RED, "SKIP": YELLOW}[status]
                    print(f"    {color}{status}{RESET}  {lang.name:<11} {DIM}{detail}{RESET}")

    print(f"\n{counts['PASS']} passed, {counts['FAIL']} failed, {counts['SKIP']} skipped")
    return 1 if counts["FAIL"] else 0


if __name__ == "__main__":
    sys.exit(main())
