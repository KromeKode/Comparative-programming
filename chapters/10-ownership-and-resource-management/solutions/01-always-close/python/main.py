import sys
from collections.abc import Iterator
from contextlib import contextmanager

MAX_OPEN = 2
MAX_LINES = 8


class DiskError(Exception):
    pass


class Handle:
    def __init__(self, disk: "Disk", name: str) -> None:
        self.disk = disk
        self.name = name

    def __iter__(self) -> Iterator[str]:
        return iter(list(self.disk.files[self.name]))

    def write(self, line: str) -> None:
        if self.disk.lines_used() == MAX_LINES:
            raise DiskError("disk full")
        self.disk.files[self.name].append(line)


class Disk:
    def __init__(self) -> None:
        self.files: dict[str, list[str]] = {}
        self.locked: set[str] = set()
        self.open_count = 0

    def lines_used(self) -> int:
        return sum(len(lines) for lines in self.files.values())

    @contextmanager
    def open(self, name: str, writing: bool = False) -> Iterator[Handle]:
        """Opens a file for a with block, and closes it however the block ends."""
        if self.open_count == MAX_OPEN:
            raise DiskError("too many open files")
        if writing:
            if name in self.locked:
                raise DiskError(f'"{name}" is read-only')
            self.files[name] = []
        elif name not in self.files:
            raise DiskError(f'no file "{name}"')
        self.open_count += 1
        print(f"open {name}")
        try:
            yield Handle(self, name)
        finally:
            self.open_count -= 1
            print(f"close {name}")


def lines(count: int) -> str:
    return "1 line" if count == 1 else f"{count} lines"


def write(disk: Disk, name: str, words: list[str]) -> None:
    with disk.open(name, writing=True) as target:
        for word in words:
            target.write(word)
    print(f"wrote {lines(len(words))}")


def cat(disk: Disk, name: str) -> None:
    with disk.open(name) as source:
        for line in source:
            print(f"  {line}")


def copy(disk: Disk, source_name: str, target_name: str) -> None:
    if source_name == target_name:
        raise DiskError(f'cannot copy "{source_name}" onto itself')
    count = 0
    with (
        disk.open(source_name) as source,
        disk.open(target_name, writing=True) as target,
    ):
        for line in source:
            target.write(line)
            count += 1
    print(f"copied {lines(count)}")


def lock(disk: Disk, name: str) -> None:
    if name not in disk.files:
        raise DiskError(f'no file "{name}"')
    disk.locked.add(name)
    print(f"locked {name}")


def takes(command: str, want: int, names: list[str]) -> None:
    if len(names) != want:
        noun = "name" if want == 1 else "names"
        raise DiskError(f"{command} takes {want} {noun}, got {len(names)}")


def run(disk: Disk, command: str, args: list[str]) -> None:
    if command == "write":
        if not args:
            raise DiskError("write needs a file name")
        write(disk, args[0], args[1:])
    elif command == "cat":
        takes(command, 1, args)
        cat(disk, args[0])
    elif command == "copy":
        takes(command, 2, args)
        copy(disk, args[0], args[1])
    elif command == "lock":
        takes(command, 1, args)
        lock(disk, args[0])
    else:
        raise DiskError(f'unknown command "{command}"')


def main() -> None:
    disk = Disk()
    for line in sys.stdin:
        parts = line.split()
        if not parts:
            continue
        try:
            run(disk, parts[0], parts[1:])
        except DiskError as error:
            print(f"error: {error}")
    print(f"files still open: {disk.open_count}")


if __name__ == "__main__":
    main()
