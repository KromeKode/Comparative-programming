# Reads CSV, as RFC 4180 describes it, and prints each record as a JSON list of strings,
# one record on each line. A field in quotes can hold commas, line breaks, and "" for a
# quote. Lines can end with "\r\n" or "\n". Every record must have as many fields as the
# first. At the first error, it prints the line and column, and the exit status is 1.
import json
import sys
from collections.abc import Iterator
from enum import Enum, auto


class State(Enum):
    RECORD_START = auto()  # nothing of this record read yet
    FIELD_START = auto()  # just after a comma
    UNQUOTED = auto()  # in a field that doesn't start with a quote
    QUOTED = auto()  # in a field that does
    QUOTE = auto()  # just after a quote in a quoted field: its end, or the first of ""
    CR = auto()  # just after a "\r" outside quotes, which needs a "\n" next


class CsvError(Exception):
    def __init__(self, line: int, column: int, message: str) -> None:
        super().__init__(f"line {line}, column {column}: {message}")


def fields(n: int) -> str:
    return "1 field" if n == 1 else f"{n} fields"


def records(text: str) -> Iterator[list[str]]:
    """The records in text, one at a time. Raises CsvError at the first error."""
    state = State.RECORD_START
    record: list[str] = []
    field: list[str] = []  # its characters so far, to join at the end
    width = 0  # how many fields the first record has, once it has ended
    line, column = 1, 0  # where char is: columns count characters, from 1
    start = (0, 0)  # where the quote that began a quoted field is, or the "\r"
    record_line = 1
    # The text, then "" for its end, so that the end goes through the machine as well.
    for char in [*text, ""]:
        column += 1
        if state is State.RECORD_START:
            record_line = line
        ended = False  # whether char ends a record
        # Each case is a state and a character; the first that fits is taken.
        match state, char:
            case State.QUOTED, "":
                raise CsvError(*start, "quoted field is never closed")
            case State.QUOTED, '"':
                state = State.QUOTE
            case State.QUOTED, _:
                field.append(char)
            case State.QUOTE, '"':
                field.append('"')
                state = State.QUOTED
            case State.CR, "\n":
                ended = True
            case State.CR, _:
                raise CsvError(*start, "a carriage return without a line feed after it")
            case State.RECORD_START, "":
                pass
            case State.RECORD_START | State.FIELD_START, '"':
                start = (line, column)
                state = State.QUOTED
            case State.UNQUOTED, '"':
                raise CsvError(line, column, "a quote in an unquoted field")
            # From here on, the state is RECORD_START, FIELD_START, UNQUOTED, or QUOTE.
            case _, ",":
                record.append("".join(field))
                field.clear()
                state = State.FIELD_START
            case _, "\n" | "":
                ended = True
            case _, "\r":
                start = (line, column)
                state = State.CR
            case State.QUOTE, _:
                raise CsvError(
                    line, column, "expected a comma or a line end after a closing quote"
                )
            case _:
                field.append(char)
                state = State.UNQUOTED
        if ended:
            record.append("".join(field))
            field.clear()
            if width == 0:
                width = len(record)
            elif len(record) != width:
                message = f"record has {fields(len(record))}, but the first has {width}"
                raise CsvError(record_line, 1, message)
            yield record
            record = []
            state = State.RECORD_START
        if char == "\n":
            line, column = line + 1, 0


def main() -> int:
    # The bytes, decoded by hand, so that each "\r" reaches the parser, whatever the
    # platform does with line ends in text mode.
    text = sys.stdin.buffer.read().decode("utf-8")
    try:
        for record in records(text):
            # ensure_ascii=False writes "é" as itself, not as "\u00e9".
            print(json.dumps(record, ensure_ascii=False, separators=(",", ":")))
    except CsvError as err:
        print(f"csv2json: {err}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
