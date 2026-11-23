# Keeps a to-do list in todo.json, in the current folder: adds tasks, lists them, marks
# them done, and removes them. ../README.md says what it should do.
import json
import os
import re
import sys
from dataclasses import asdict, dataclass

FILE = "todo.json"
VERSION = 1
USAGE = """usage: todo add TEXT...
       todo list
       todo done ID
       todo remove ID"""


class Problem(Exception):
    """A problem with todo.json, which stops the program with status 2."""


@dataclass
class Task:
    id: int
    text: str
    done: bool


def reject(name: str) -> object:
    raise ValueError(f"{name} is not JSON")  # json.loads reads NaN and Infinity


def task_from_json(value: object) -> Task:
    match value:
        # A dict pattern matches a dict with these keys, and **rest takes any others.
        # bool is a kind of int in Python, so int(number) matches true too, unless the
        # guard stops it.
        case {"id": int(number), "text": str(text), "done": bool(done), **rest} if (
            not rest and not isinstance(number, bool) and number >= 1
        ):
            return Task(number, text, done)
        case _:
            raise Problem("not a to-do list")


def read_tasks() -> list[Task]:
    """The tasks in todo.json, or none if there's no todo.json."""
    try:
        with open(FILE, "rb") as file:
            data = file.read()
    except FileNotFoundError:
        return []
    except OSError as error:
        raise Problem(error.strerror) from None
    try:
        value = json.loads(data.decode("utf-8"), parse_constant=reject)
    except ValueError:
        raise Problem("not valid JSON") from None
    if not isinstance(value, dict):
        raise Problem("not a to-do list")
    # A later version of todo may keep anything else in the file, so the version comes
    # first. type(), not isinstance, so that true isn't taken for 1.
    version = value.get("version")
    if type(version) in (int, float) and version != VERSION:
        raise Problem(f"unknown version {version}")
    tasks = value.get("tasks")
    if type(version) is not int or len(value) != 2 or not isinstance(tasks, list):
        raise Problem("not a to-do list")
    result = [task_from_json(task) for task in tasks]
    if len({task.id for task in result}) != len(result):
        raise Problem("not a to-do list")  # two tasks with one id
    return result


def write_tasks(tasks: list[Task]) -> None:
    """Writes the tasks to todo.json. It writes a new file and renames it over the old
    one, which replaces it in one step, so that a crash leaves the old list or the new
    one, and never half of one."""
    value = {"version": VERSION, "tasks": [asdict(task) for task in tasks]}
    text = json.dumps(value, indent=2, ensure_ascii=False) + "\n"
    temporary = FILE + ".tmp"
    try:
        with open(temporary, "w", encoding="utf-8") as file:
            file.write(text)
            file.flush()
            os.fsync(file.fileno())  # onto the disk before the rename
        os.replace(temporary, FILE)
    except OSError as error:
        if os.path.exists(temporary):
            os.remove(temporary)
        raise Problem(error.strerror) from None


def run(command: str, args: list[str]) -> int:
    """Carries out a command whose arguments have been checked, and returns the status."""
    tasks = read_tasks()
    if command == "add":
        added = Task(max((task.id for task in tasks), default=0) + 1, " ".join(args), False)
        tasks.append(added)
        write_tasks(tasks)
        print(f"added {added.id}: {added.text}")
        return 0
    if command == "list":
        for task in tasks:
            print(f"{task.id} [{'x' if task.done else ' '}] {task.text}")
        return 0
    number = int(args[0])
    found = next((task for task in tasks if task.id == number), None)
    if found is None:
        print(f"todo: no task {args[0]}", file=sys.stderr)
        return 1
    if command == "done":
        found.done = True
        write_tasks(tasks)
        print(f"done {found.id}: {found.text}")
    else:
        tasks.remove(found)
        write_tasks(tasks)
        print(f"removed {found.id}: {found.text}")
    return 0


def main() -> int:
    command, args = (sys.argv[1], sys.argv[2:]) if len(sys.argv) > 1 else ("", [])
    # The command line is checked before todo.json is read. An ID is digits: int()
    # would take " 1", "+1", and "1_000" too.
    ok = (
        (command == "add" and len(args) > 0)
        or (command == "list" and len(args) == 0)
        or (command in ("done", "remove") and len(args) == 1
            and re.fullmatch("[0-9]+", args[0]) is not None)
    )
    if not ok:
        print(USAGE, file=sys.stderr)
        return 2
    try:
        return run(command, args)
    except Problem as problem:
        print(f"todo: {FILE}: {problem}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
