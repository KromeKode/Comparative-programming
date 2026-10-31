import sys
from dataclasses import dataclass


# None means "not set", which is different from every real value, the default included.
@dataclass
class Config:
    host: str | None = None
    port: int | None = None
    timeout: int | None = None


def parse_number(text: str, low: int, high: int) -> int | None:
    """Returns the number if text is digits only and from low to high, or else None."""
    if text == "" or any(c not in "0123456789" for c in text):
        return None
    value = int(text)
    return value if low <= value <= high else None


def main() -> None:
    config = Config()
    for number, line in enumerate(sys.stdin, start=1):
        line = line.strip()
        if line == "" or line.startswith("#"):
            continue
        key, equals, value = line.partition("=")
        if equals == "":
            print(f"line {number}: expected key = value")
            continue
        key, value = key.strip(), value.strip()
        match key:
            case "host":
                config.host = value if value != "" else None
            case "port":
                port = parse_number(value, 1, 65535)
                if value == "":
                    config.port = None
                elif port is None:
                    print(f'line {number}: bad port "{value}"')
                else:
                    config.port = port
            case "timeout":
                timeout = parse_number(value, 1, 86400)
                if value == "":
                    config.timeout = None
                elif timeout is None:
                    print(f'line {number}: bad timeout "{value}"')
                else:
                    config.timeout = timeout
            case _:
                print(f'line {number}: unknown key "{key}"')

    # Each None turns into its default here, at the last moment.
    host = config.host if config.host is not None else "localhost (default)"
    port = config.port if config.port is not None else "8080 (default)"
    timeout = f"{config.timeout}s" if config.timeout is not None else "none"
    print(f"host: {host}")
    print(f"port: {port}")
    print(f"timeout: {timeout}")


if __name__ == "__main__":
    main()
