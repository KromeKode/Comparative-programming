# Prints how one JSON file differs from another: what was removed, what was added, and
# what changed. ../README.md says what it should print.
import json
import re
import sys

USAGE = "usage: jsondiff OLD NEW"
IDENTIFIER = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")  # \w would match é too


def reject(name: str) -> object:
    # json.loads reads NaN, Infinity, and -Infinity, which JSON doesn't have, and calls
    # parse_constant for them.
    raise ValueError(f"{name} is not JSON")


def read_json(path: str) -> object:
    """The value in a file. Raises OSError if it can't be read, ValueError if it isn't
    JSON: json.JSONDecodeError and UnicodeDecodeError are both kinds of ValueError."""
    with open(path, "rb") as file:
        data = file.read()
    return json.loads(data.decode("utf-8"), parse_constant=reject)


def compact(value: object) -> str:
    # sort_keys sorts by the characters' codes; ensure_ascii=False writes é as it is.
    return json.dumps(value, ensure_ascii=False, separators=(",", ":"), sort_keys=True)


def step(key: str) -> str:
    """The part of a path that leads to a key: .name, or ["first name"]."""
    return f".{key}" if IDENTIFIER.fullmatch(key) else f"[{compact(key)}]"


def diff(path: str, before: object, after: object, out: list[str]) -> None:
    """Adds a line to out for each difference between before and after."""
    if isinstance(before, dict) and isinstance(after, dict):
        for key in sorted(before.keys() | after.keys()):
            here = path + step(key)
            if key not in after:
                out.append(f"- {here}: {compact(before[key])}")
            elif key not in before:
                out.append(f"+ {here}: {compact(after[key])}")
            else:
                diff(here, before[key], after[key], out)
    elif isinstance(before, list) and isinstance(after, list):
        for i in range(max(len(before), len(after))):
            here = f"{path}[{i}]"
            if i >= len(after):
                out.append(f"- {here}: {compact(before[i])}")
            elif i >= len(before):
                out.append(f"+ {here}: {compact(after[i])}")
            else:
                diff(here, before[i], after[i], out)
    # In Python True == 1 and 0 == False, so the types need comparing too.
    elif type(before) is not type(after) or before != after:
        out.append(f"~ {path}: {compact(before)} -> {compact(after)}")


def main() -> int:
    if len(sys.argv) != 3:
        print(USAGE, file=sys.stderr)
        return 2
    values = []
    for path in sys.argv[1:]:
        try:
            values.append(read_json(path))
        except OSError as error:
            print(f"jsondiff: {path}: {error.strerror}", file=sys.stderr)
            return 2
        except ValueError:
            print(f"jsondiff: {path}: not valid JSON", file=sys.stderr)
            return 2
    lines: list[str] = []
    diff("$", values[0], values[1], lines)
    # UTF-8 whatever the locale, as the files are.
    sys.stdout.buffer.write("".join(line + "\n" for line in lines).encode())
    return 1 if lines else 0


if __name__ == "__main__":
    sys.exit(main())
