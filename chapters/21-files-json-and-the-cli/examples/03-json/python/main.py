# Reads a JSON file of books and prints a report on them, as JSON: how many there are,
# the oldest, the books with each tag, and the books with none. json.load gives dicts
# and lists, and match checks that each book has the fields it needs.
import json
import sys
from dataclasses import dataclass


@dataclass
class Book:
    title: str
    author: str
    year: int
    tags: list[str]


def book_from_json(value: object) -> Book:
    match value:
        # A dict pattern matches a dict with these keys, and **rest takes any others.
        # bool is a kind of int in Python, so int(year) matches true too, unless the
        # guard stops it.
        case {
            "title": str(title),
            "author": str(author),
            "year": int(year),
            **rest,
        } if not isinstance(year, bool):
            tags = rest.get("tags", [])
            if not isinstance(tags, list) or not all(isinstance(t, str) for t in tags):
                raise ValueError(f"{title}: tags must be a list of strings")
            return Book(title, author, year, tags)
        case _:
            raise ValueError("each book needs a title, an author, and a year")


def read_books(path: str) -> list[Book]:
    with open(path, encoding="utf-8") as file:
        data = json.load(
            file
        )  # raises json.JSONDecodeError, a ValueError, if it isn't JSON
    match data:
        case {"books": list(books)}:
            return [book_from_json(book) for book in books]
        case _:
            raise ValueError("expected an object with a list of books")


def report(books: list[Book]) -> dict[str, object]:
    # min gives the first of the oldest when two have the same year.
    oldest = min(books, key=lambda book: book.year, default=None)
    tags: dict[str, list[str]] = {}
    for book in books:
        for tag in book.tags:
            tags.setdefault(tag, []).append(book.title)
    return {
        "count": len(books),
        # json.dumps writes None as null.
        "oldest": f"{oldest.title}, by {oldest.author}" if oldest else None,
        "tags": dict(sorted(tags.items())),  # a dict keeps the order its keys went in
        "untagged": [book.title for book in books if not book.tags],
    }


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: books FILE", file=sys.stderr)
        return 2
    path = sys.argv[1]
    try:
        books = read_books(path)
    except OSError as error:
        print(f"books: {path}: {error.strerror}", file=sys.stderr)
        return 1
    except ValueError as error:
        print(f"books: {path}: {error}", file=sys.stderr)
        return 1
    # ensure_ascii=False writes "ñ" as it is, instead of as "ñ".
    print(json.dumps(report(books), indent=2, ensure_ascii=False))
    return 0


if __name__ == "__main__":
    sys.exit(main())
