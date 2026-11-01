import sys
from dataclasses import dataclass


@dataclass
class Config:
    name: str
    port: int
    debug: bool
    timeout: int


# The value parsers raise ValueProblem; load wraps it in a BadValue.
class ValueProblem(Exception):
    pass


class ConfigError(Exception):
    """The base class for every problem load can find in a file."""

    kind = ""


class SyntaxProblem(ConfigError):
    kind = "syntax"

    def __init__(self, line: int):
        super().__init__(f'line {line}: expected "key = value"')


class UnknownKey(ConfigError):
    kind = "unknown key"

    def __init__(self, line: int, key: str):
        super().__init__(f'line {line}: unknown key "{key}"')


class DuplicateKey(ConfigError):
    kind = "duplicate key"

    def __init__(self, line: int, key: str, first: int):
        super().__init__(f'line {line}: "{key}" is already set on line {first}')


class BadValue(ConfigError):
    kind = "bad value"

    def __init__(self, line: int, key: str, problem: ValueProblem):
        super().__init__(f"line {line}: {key}: {problem}")


class MissingKey(ConfigError):
    kind = "missing key"

    def __init__(self, key: str):
        super().__init__(f'missing key "{key}"')


KINDS = ["syntax", "unknown key", "duplicate key", "bad value", "missing key"]


def parse_text(text: str) -> str:
    if text == "":
        raise ValueProblem("must not be empty")
    return text


def parse_whole(text: str, low: int, high: int) -> int:
    if text == "" or any(c not in "0123456789" for c in text):
        raise ValueProblem(f'"{text}" is not a whole number')
    value = int(text)
    if not low <= value <= high:
        raise ValueProblem(f"{text} is not between {low} and {high}")
    return value


def parse_bool(text: str) -> bool:
    if text not in ("true", "false"):
        raise ValueProblem(f'"{text}" is not true or false')
    return text == "true"


PARSERS = {
    "name": parse_text,
    "port": lambda text: parse_whole(text, 1, 65535),
    "debug": parse_bool,
    "timeout": lambda text: parse_whole(text, 1, 3600),
}
DEFAULTS = {"debug": False, "timeout": 30}


def load(lines: list[str]) -> Config:
    """Return the config, or raise an ExceptionGroup of every problem found."""
    values: dict[str, object] = dict(DEFAULTS)
    first_line: dict[str, int] = {}
    errors: list[ConfigError] = []
    for number, line in enumerate(lines, start=1):
        stripped = line.strip(" ")
        if stripped == "" or stripped.startswith("#"):
            continue
        key, equals, text = stripped.partition("=")
        key, text = key.strip(" "), text.strip(" ")
        if not equals or key == "":
            errors.append(SyntaxProblem(number))
        elif key not in PARSERS:
            errors.append(UnknownKey(number, key))
        elif key in first_line:
            errors.append(DuplicateKey(number, key, first_line[key]))
        else:
            first_line[key] = number
            try:
                values[key] = PARSERS[key](text)
            except ValueProblem as problem:
                errors.append(BadValue(number, key, problem))
    for key in ["name", "port"]:
        if key not in first_line:
            errors.append(MissingKey(key))
    if errors:
        raise ExceptionGroup("the config has errors", errors)
    return Config(**values)  # type: ignore[arg-type]


def split_files(text: str) -> list[tuple[str, list[str]]]:
    files: list[tuple[str, list[str]]] = []
    for line in text.splitlines():
        if line.startswith("== "):
            files.append((line[3:], []))
        elif files:
            files[-1][1].append(line)
    return files


def main() -> None:
    files = split_files(sys.stdin.read())
    counts = dict.fromkeys(KINDS, 0)
    loaded = 0
    for name, lines in files:
        try:
            config = load(lines)
        except ExceptionGroup as group:
            count = len(group.exceptions)
            print(f"{name}: {count} error{'' if count == 1 else 's'}")
            for error in group.exceptions:
                print(f"  {error}")
                assert isinstance(error, ConfigError)
                counts[error.kind] += 1
            continue
        loaded += 1
        print(f"{name}: ok")
        print(f"  name = {config.name}")
        print(f"  port = {config.port}")
        print(f"  debug = {'true' if config.debug else 'false'}")
        print(f"  timeout = {config.timeout}")
    print(f"files loaded: {loaded} of {len(files)}")
    print(", ".join(f"{kind}: {counts[kind]}" for kind in KINDS))


if __name__ == "__main__":
    main()
