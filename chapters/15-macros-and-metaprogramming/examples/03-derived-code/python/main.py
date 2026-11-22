from dataclasses import dataclass, fields, replace


# @dataclass reads the annotated names in the class body when the class is
# created, and writes __init__, __repr__, and __eq__ from them. order=True
# adds < and the rest, comparing the fields in order. frozen=True makes the
# fields read-only, which also gives the class a __hash__ for sets.
@dataclass(frozen=True, order=True)
class Book:
    title: str
    author: str
    year: int
    in_print: bool


def main() -> None:
    print("fields:", " ".join(field.name for field in fields(Book)))

    dune = Book("Dune", "Frank Herbert", 1965, True)
    print(f"dune: {dune}")

    copy = replace(dune)
    print(f"copy == dune: {copy == dune}")

    # replace makes a new Book with the fields it names changed.
    reissue = replace(dune, year=1966)
    print(f"reissue: {reissue}")
    print(f"reissue == dune: {reissue == dune}")
    print(f"dune < reissue: {dune < reissue}")

    print(f"distinct books: {len({dune, copy, reissue})}")


if __name__ == "__main__":
    main()
