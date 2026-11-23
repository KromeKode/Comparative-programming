# Reads a trace of function calls and prints a flat profile: for each function, how
# many calls it got, its total time, and its self time. See ../README.md.
import sys
from dataclasses import dataclass, field


@dataclass
class Function:
    calls: int = 0
    total: int = 0
    self_time: int = 0
    running: int = 0  # how many of its calls are running now
    outermost_start: int = 0  # when the outermost of them started


@dataclass
class Call:
    name: str
    start: int
    in_callees: int = field(default=0)  # time spent in the functions it called


class TraceError(Exception):
    pass


def profile(lines: list[str]) -> dict[str, Function]:
    functions: dict[str, Function] = {}
    stack: list[Call] = []
    now = 0
    for number, line in enumerate(lines, start=1):
        fields = line.replace("\t", " ").split(" ")
        fields = [f for f in fields if f]
        if not fields:
            continue
        if (
            len(fields) != 3
            or not (
                fields[0].isascii() and fields[0].isdigit() and len(fields[0]) <= 12
            )
            or fields[1] not in ("enter", "exit")
            or not all("!" <= c <= "~" for c in fields[2])
        ):
            raise TraceError(
                f"line {number}: expected TIME enter NAME or TIME exit NAME"
            )
        time, event, name = int(fields[0]), fields[1], fields[2]
        if time < now:
            raise TraceError(f"line {number}: time goes back from {now} to {time}")
        now = time

        if event == "enter":
            function = functions.setdefault(name, Function())
            function.calls += 1
            if function.running == 0:
                function.outermost_start = time
            function.running += 1
            stack.append(Call(name, time))
            continue

        if not stack:
            raise TraceError(f"line {number}: exit from {name}, but nothing is running")
        if stack[-1].name != name:
            raise TraceError(
                f"line {number}: exit from {name}, but the innermost call is to {stack[-1].name}"
            )
        call = stack.pop()
        duration = time - call.start
        function = functions[name]
        function.self_time += duration - call.in_callees
        function.running -= 1
        # A recursive call's time is already in the total of the call around it.
        if function.running == 0:
            function.total += time - function.outermost_start
        if stack:
            stack[-1].in_callees += duration

    if stack:
        raise TraceError(f"end of trace: {stack[-1].name} is still running")
    return functions


def main() -> None:
    try:
        functions = profile(sys.stdin.read().splitlines())
    except TraceError as error:
        print(f"error: {error}")
        return
    if not functions:
        print("no calls")
    for name, f in sorted(
        functions.items(), key=lambda item: (-item[1].self_time, item[0])
    ):
        print(name, f.calls, f.total, f.self_time)


if __name__ == "__main__":
    main()
