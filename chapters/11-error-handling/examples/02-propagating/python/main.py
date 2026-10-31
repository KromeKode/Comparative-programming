# Pretend files, so the example does not touch the disk.
FILES = {"good.txt": "1\n2\n3", "bad.txt": "4\nfive\n6"}


# One exception class per kind of failure, so callers can catch just one.
class NotFound(Exception):
    pass


class NotANumber(Exception):
    pass


class BadLine(Exception):
    pass


def read_file(name: str) -> str:
    if name not in FILES:
        raise NotFound(f"{name}: no such file")
    return FILES[name]


def parse_number(text: str) -> int:
    try:
        return int(text)
    except ValueError:
        # Replace int's ValueError with our own; "from None" drops it.
        raise NotANumber(f'"{text}" is not a number') from None


def sum_file(name: str) -> int:
    text = read_file(name)  # a NotFound passes straight through
    total = 0
    for line, number in enumerate(text.splitlines(), start=1):
        try:
            total += parse_number(number)
        except NotANumber as error:
            # Raise a new exception and keep the old one as its __cause__.
            raise BadLine(f"{name}: line {line}") from error
    return total


def sum_all(names: list[str]) -> int:
    """A missing file counts as 0, but any other error stops the sum."""
    total = 0
    for name in names:
        try:
            total += sum_file(name)
        except NotFound:
            pass
    return total


def describe(error: BaseException) -> str:
    """Join an exception's message with the messages of its causes."""
    parts = []
    cause: BaseException | None = error
    while cause is not None:
        parts.append(str(cause))
        cause = cause.__cause__
    return ": ".join(parts)


def show(label, function, *args):
    try:
        print(f"  {label} -> {function(*args)}")
    except Exception as error:
        print(f"  {label} -> error: {describe(error)}")


print("one file at a time:")
for name in ["good.txt", "bad.txt", "missing.txt"]:
    show(name, sum_file, name)
print("several files, where a missing file counts as 0:")
show("good.txt + missing.txt", sum_all, ["good.txt", "missing.txt"])
show("good.txt + bad.txt", sum_all, ["good.txt", "bad.txt"])
