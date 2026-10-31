import sys
from abc import ABC, abstractmethod


class EditError(Exception):
    pass


class Command(ABC):
    """An edit that knows how to do itself and how to undo itself."""

    @abstractmethod
    def apply(self, lines: list[str]) -> None: ...

    @abstractmethod
    def undo(self, lines: list[str]) -> None: ...

    @abstractmethod
    def describe(self) -> str: ...


def check_line(lines: list[str], number: int) -> None:
    if not 1 <= number <= len(lines):
        raise EditError(f"no line {number}")


class Add(Command):
    def __init__(self, text: str) -> None:
        self.text = text

    def apply(self, lines: list[str]) -> None:
        lines.append(self.text)

    def undo(self, lines: list[str]) -> None:
        lines.pop()

    def describe(self) -> str:
        return f'add "{self.text}"'


class Delete(Command):
    def __init__(self, number: int) -> None:
        self.number = number
        self.removed = ""

    def apply(self, lines: list[str]) -> None:
        check_line(lines, self.number)
        self.removed = lines.pop(self.number - 1)

    def undo(self, lines: list[str]) -> None:
        lines.insert(self.number - 1, self.removed)

    def describe(self) -> str:
        return f"delete line {self.number}"


class Replace(Command):
    def __init__(self, number: int, text: str) -> None:
        self.number = number
        self.text = text
        self.old = ""

    def apply(self, lines: list[str]) -> None:
        check_line(lines, self.number)
        self.old = lines[self.number - 1]
        lines[self.number - 1] = self.text

    def undo(self, lines: list[str]) -> None:
        lines[self.number - 1] = self.old

    def describe(self) -> str:
        return f'replace line {self.number} with "{self.text}"'


class Editor:
    def __init__(self) -> None:
        self.lines: list[str] = []
        self.done: list[Command] = []
        self.undone: list[Command] = []

    def run(self, command: Command) -> None:
        command.apply(self.lines)  # if this raises, nothing is recorded
        self.done.append(command)
        self.undone.clear()

    def undo(self) -> None:
        if not self.done:
            raise EditError("nothing to undo")
        command = self.done.pop()
        command.undo(self.lines)
        self.undone.append(command)
        print(f"undo: {command.describe()}")

    def redo(self) -> None:
        if not self.undone:
            raise EditError("nothing to redo")
        command = self.undone.pop()
        command.apply(self.lines)
        self.done.append(command)
        print(f"redo: {command.describe()}")

    def show(self) -> None:
        if not self.lines:
            print("(empty)")
        for number, line in enumerate(self.lines, start=1):
            print(f"{number}: {line}")


def main() -> None:
    editor = Editor()
    for line in sys.stdin:
        try:
            match line.split():
                case []:
                    continue
                case ["add", *words]:
                    editor.run(Add(" ".join(words)))
                case ["delete", number]:
                    editor.run(Delete(int(number)))
                case ["replace", number, *words]:
                    editor.run(Replace(int(number), " ".join(words)))
                case ["undo"]:
                    editor.undo()
                case ["redo"]:
                    editor.redo()
                case ["show"]:
                    editor.show()
        except EditError as error:
            print(f"error: {error}")


if __name__ == "__main__":
    main()
