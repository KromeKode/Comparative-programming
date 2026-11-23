# Removes repeated lines from each file named on the command line, keeping the first of
# each, and rewrites the file in place. It writes a new file next to the old one and
# renames it over the old one, so that a crash halfway leaves the old file whole.
import os
import shutil
import sys
import tempfile


def dedup(path: str) -> int:
    """Rewrites the file without its repeated lines, and returns how many there were."""
    # newline="" keeps "\r\n" as it is; without it, Python turns it into "\n".
    with open(path, encoding="utf-8", newline="") as file:
        text = file.read()
    lines = text.split("\n")
    if lines[-1] == "":  # the file ended with a newline, or was empty
        lines.pop()
    kept = list(dict.fromkeys(lines))  # a dict keeps the first of each, in order
    removed = len(lines) - len(kept)
    if removed == 0:
        return 0

    folder = os.path.dirname(path) or "."
    # A new file in the same folder, so that the rename can't cross to another disk.
    fd, temporary = tempfile.mkstemp(dir=folder, prefix=".dedup-")
    try:
        with open(fd, "w", encoding="utf-8", newline="") as file:
            file.write("".join(line + "\n" for line in kept))
            file.flush()
            os.fsync(file.fileno())  # onto the disk before the rename
        shutil.copymode(path, temporary)  # mkstemp makes it readable by its owner only
        os.replace(temporary, path)
    except BaseException:
        os.remove(temporary)
        raise
    return removed


def main() -> int:
    if len(sys.argv) < 2:
        print("usage: dedup FILE...", file=sys.stderr)
        return 2
    status = 0
    for path in sys.argv[1:]:
        try:
            removed = dedup(path)
        except OSError as error:  # strerror is the system's message, without the path
            print(f"dedup: {path}: {error.strerror}", file=sys.stderr)
            status = 1
            continue
        except UnicodeDecodeError:
            print(f"dedup: {path}: not UTF-8 text", file=sys.stderr)
            status = 1
            continue
        print(f"{path}: removed {removed} line{'' if removed == 1 else 's'}")
    return status


if __name__ == "__main__":
    sys.exit(main())
