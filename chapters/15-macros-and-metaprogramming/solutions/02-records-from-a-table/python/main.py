import re
import sys
from collections.abc import Callable
from dataclasses import dataclass, fields
from typing import Any


# The fields are written once, here. Everything below finds them with
# dataclasses.fields, which lists each one's name, type, and default.
@dataclass
class Book:
    title: str = ""
    author: str = ""
    year: int = 0
    pages: int = 0
    in_print: bool = False


class BadValue(Exception):
    pass


def parse_text(text: str) -> str:
    return text


def parse_whole(text: str) -> int:
    if not re.fullmatch(r"-?[0-9]+", text):
        raise BadValue(f'"{text}" is not a whole number')
    return int(text)


def parse_bool(text: str) -> bool:
    if text not in ("true", "false"):
        raise BadValue(f'"{text}" is not true or false')
    return text == "true"


# One parser for each type a field can have, found by the field's type.
PARSERS: dict[object, Callable[[str], Any]] = {
    str: parse_text,
    int: parse_whole,
    bool: parse_bool,
}


def show(value: object) -> str:
    if isinstance(value, bool):
        return "true" if value else "false"
    if isinstance(value, str):
        return f'"{value}"'
    return str(value)


def describe(book: Book) -> str:
    parts = [
        f"{field.name}={show(getattr(book, field.name))}" for field in fields(Book)
    ]
    return f"{type(book).__name__}({', '.join(parts)})"


def plural(count: int, word: str) -> str:
    return f"{count} {word}" + ("" if count == 1 else "s")


def main() -> None:
    lines = [line for line in sys.stdin.read().split("\n") if line.strip()]
    if not lines:
        return
    columns = [name.strip() for name in lines[0].split(",")]
    by_name = {field.name: field for field in fields(Book)}
    for i, name in enumerate(columns):
        if name not in by_name:
            print(f"unknown column: {name}")
            return
        if name in columns[:i]:
            print(f"repeated column: {name}")
            return

    books, bad = 0, 0
    for row, line in enumerate(lines[1:], start=1):
        values = [value.strip() for value in line.split(",")]
        try:
            if len(values) != len(columns):
                raise BadValue(f"expected {len(columns)} values, got {len(values)}")
            book = Book()
            for name, text in zip(columns, values):
                parse = PARSERS[by_name[name].type]
                try:
                    # setattr sets a field chosen by its name, as the program runs.
                    setattr(book, name, parse(text))
                except BadValue as error:
                    raise BadValue(f"{name}: {error}") from None
        except BadValue as error:
            print(f"row {row}: {error}")
            bad += 1
            continue
        print(describe(book))
        books += 1
    print(f"{plural(books, 'book')}, {plural(bad, 'bad row')}")


if __name__ == "__main__":
    main()
