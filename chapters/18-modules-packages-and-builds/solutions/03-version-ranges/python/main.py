"""Exercise 18.3: version ranges. The highest version that each range allows.

The versions and ranges are semver.py's business; this file reads the input and
prints the answers."""

import sys

from semver import highest, parse_range, parse_version


def main() -> None:
    lines = sys.stdin.read().splitlines()
    if not lines:
        return
    versions = []
    for word in lines[0].split():
        version = parse_version(word)
        if version is None:
            print(f"error: line 1: {word} isn't a version")
            return
        versions.append(version)
    if not versions:
        print("error: line 1 must list the versions")
        return
    for number, raw in enumerate(lines[1:], start=2):
        line = raw.strip()
        if not line:
            continue
        try:
            allowed = parse_range(line)
        except ValueError as error:
            print(f"error: line {number}: {error}")
            continue
        best = highest(versions, allowed)
        print(f"{line} -> {best if best else 'none'}")


if __name__ == "__main__":
    main()
