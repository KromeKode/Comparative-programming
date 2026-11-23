# Prints the lines that contain a pattern, from each file named on the command line, or
# from stdin. ../README.md says what it should print.
import os
import sys
from typing import BinaryIO

USAGE = "usage: search [-icnv] PATTERN [FILE...]"
STDIN_NAME = "(standard input)"


def usage_error(message: str) -> int:
    print(f"search: {message}\n{USAGE}", file=sys.stderr)
    return 2


def search(
    data: bytes, name: str | None, pattern: bytes, options: set[str], out: BinaryIO
) -> int:
    """Prints the lines of data that are selected, and returns how many there are."""
    lines = data.split(b"\n")
    if lines[-1] == b"":  # the data ended with a newline, or was empty
        lines.pop()
    if "i" in options:
        pattern = pattern.lower()  # bytes.lower changes only A to Z
    prefix = b"" if name is None else os.fsencode(name) + b":"
    count = 0
    for number, line in enumerate(lines, 1):
        found = pattern in (line.lower() if "i" in options else line)
        if found == ("v" in options):
            continue
        count += 1
        if "c" not in options:
            line_number = f"{number}:".encode() if "n" in options else b""
            out.write(prefix + line_number + line + b"\n")
    if "c" in options:
        out.write(prefix + f"{count}\n".encode())
    return count


def main() -> int:
    args = sys.argv[1:]
    options: set[str] = set()
    # Options come first; the first argument that isn't one is the pattern.
    while args and args[0].startswith("-") and args[0] != "-":
        arg = args.pop(0)
        if arg == "--":
            break
        for letter in arg[1:]:
            if letter not in "icnv":
                return usage_error(f"unknown option -{letter}")
            options.add(letter)
    if not args:
        return usage_error("no pattern")
    # sys.argv holds str, which Python decoded from the bytes it was given; os.fsencode
    # turns one back into the same bytes, even ones that aren't UTF-8.
    pattern = os.fsencode(args[0])
    files = args[1:] or ["-"]
    several = len(files) > 1
    out = sys.stdout.buffer  # bytes, not str, so that every byte is written as it is

    selected = 0
    status = 0
    for file in files:
        name = STDIN_NAME if file == "-" else file
        try:
            if file == "-":
                data = sys.stdin.buffer.read()
            else:
                with open(file, "rb") as f:  # "rb": bytes, with nothing decoded
                    data = f.read()
        except OSError as error:
            out.flush()  # so that stdout and stderr come out in order on a terminal
            print(f"search: {file}: {error.strerror}", file=sys.stderr)
            status = 2
            continue
        selected += search(data, name if several else None, pattern, options, out)
    if status == 0 and selected == 0:
        status = 1
    return status


if __name__ == "__main__":
    sys.exit(main())
