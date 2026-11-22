import heapq
import sys
from collections import deque
from dataclasses import dataclass, field


@dataclass(frozen=True)
class Print:
    text: str


@dataclass(frozen=True)
class Sleep:
    duration: int


@dataclass(frozen=True)
class Wait:
    name: str


type Step = Print | Sleep | Wait


@dataclass(eq=False)
class Task:
    name: str
    steps: list[Step] = field(default_factory=list)
    next: int = 0  # the index of the step to run when it runs again
    done: bool = False
    waiting_for: str = ""
    waiters: list["Task"] = field(default_factory=list)  # in the order they began


def parse(lines: list[str]) -> list[Task]:
    """Returns the tasks, and prints an error for each line with a problem."""
    # A wait can name a task defined further down, so the names come first.
    names = {
        line.strip().partition(" ")[2].strip()
        for line in lines
        if line.split()[:1] == ["task"]
    }
    tasks: list[Task] = []
    for number, line in enumerate(lines, start=1):
        command, _, rest = line.strip().partition(" ")
        rest = rest.strip()
        if not command:
            continue
        if command == "task":
            tasks.append(Task(rest))
            continue
        if not tasks:
            print(f"error: line {number}: a step before any task")
        elif command == "print":
            tasks[-1].steps.append(Print(rest))
        elif command == "sleep" and rest.isascii() and rest.isdigit():
            tasks[-1].steps.append(Sleep(int(rest)))
        elif command == "sleep":
            print(f"error: line {number}: sleep takes a whole number")
        elif command == "wait" and rest in names:
            tasks[-1].steps.append(Wait(rest))
        elif command == "wait":
            print(f"error: line {number}: no task named {rest}")
        else:
            print(f"error: line {number}: unknown step: {command}")
    return tasks


class Loop:
    def __init__(self, tasks: list[Task]) -> None:
        self.tasks = tasks
        self.by_name = {task.name: task for task in tasks}
        self.time = 0
        self.ready = deque(tasks)  # the tasks that can run now, in order
        # The sleeping tasks, as (time to wake, order of the sleep, task). The
        # heap's smallest tuple is the one to wake first.
        self.sleeping: list[tuple[int, int, Task]] = []
        self.sleeps = 0

    def run_until_stopped(self, task: Task) -> None:
        """Runs a task until it sleeps, waits, or ends."""
        while task.next < len(task.steps):
            step = task.steps[task.next]
            task.next += 1
            match step:
                case Print(text):
                    print(f"[{self.time}] {task.name}: {text}")
                case Sleep(0):
                    self.ready.append(task)
                    return
                case Sleep(duration):
                    heapq.heappush(
                        self.sleeping, (self.time + duration, self.sleeps, task)
                    )
                    self.sleeps += 1
                    return
                case Wait(name) if not self.by_name[name].done:
                    task.waiting_for = name
                    self.by_name[name].waiters.append(task)
                    return
                case Wait():
                    pass  # it has finished already, so carry on
        task.done = True
        print(f"[{self.time}] {task.name}: done")
        self.ready.extend(task.waiters)

    def run(self) -> None:
        while True:
            while self.ready:
                self.run_until_stopped(self.ready.popleft())
            if not self.sleeping:
                break
            # Nothing can run now, so the clock jumps to the next wake.
            self.time = self.sleeping[0][0]
            while self.sleeping and self.sleeping[0][0] == self.time:
                self.ready.append(heapq.heappop(self.sleeping)[2])
        stuck = [
            f"{t.name} waits for {t.waiting_for}" for t in self.tasks if not t.done
        ]
        if stuck:
            print(f"[{self.time}] deadlock: {', '.join(stuck)}")
        else:
            print(f"[{self.time}] all done")


def main() -> None:
    Loop(parse(sys.stdin.read().splitlines())).run()


if __name__ == "__main__":
    main()
