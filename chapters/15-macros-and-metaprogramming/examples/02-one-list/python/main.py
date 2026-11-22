from enum import Enum


# No macro is needed. A class statement runs its body as code, and Enum's
# metaclass then turns each name in it into a member that knows its name.
class Status(Enum):
    OK = 200, "OK"
    MOVED = 301, "Moved Permanently"
    NOT_FOUND = 404, "Not Found"
    TEAPOT = 418, "I'm a teapot"
    SERVER_ERROR = 500, "Internal Server Error"

    # Each member's value is unpacked into these arguments.
    def __init__(self, code: int, text: str) -> None:
        self.code = code
        self.text = text


def find_code(code: int) -> Status | None:
    return next((status for status in Status if status.code == code), None)


def main() -> None:
    print(f"{len(Status)} statuses")
    for status in Status:
        print(f"{status.code} {status.name}: {status.text}")
    for code in [404, 999]:
        found = find_code(code)
        print(f"lookup {code}: {found.text if found else 'unknown'}")
    for name in ["TEAPOT", "BREWING"]:
        # Status[name] looks a member up by its name, as the program runs.
        if name in Status.__members__:
            print(f"lookup {name}: {Status[name].code}")
        else:
            print(f"lookup {name}: unknown")


if __name__ == "__main__":
    main()
