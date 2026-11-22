"""Exercise 18.2: build order. The order to build modules in, and what to rebuild."""

import heapq
import re
import sys

NAME = re.compile(r"[A-Za-z0-9_./-]+")


def main() -> None:
    imports: dict[str, list[str]] = {}
    defined_on: dict[str, int] = {}
    queries: list[tuple[int, str]] = []
    for number, raw in enumerate(sys.stdin.read().splitlines(), start=1):
        line = raw.strip()
        if not line:
            continue
        if ":" in line:
            name, _, rest = line.partition(":")
            name = name.strip()
            deps = rest.split()
            if not NAME.fullmatch(name) or not all(NAME.fullmatch(d) for d in deps):
                print(f"error: line {number}: a module is NAME: IMPORTS")
                return
            if name in imports:
                print(f"error: line {number}: {name} is already defined on line {defined_on[name]}")
                return
            imports[name] = list(dict.fromkeys(deps))
            defined_on[name] = number
            continue
        words = line.split()
        if len(words) == 2 and words[0] == "rebuild":
            queries.append((number, words[1]))
            continue
        print(f"error: line {number}: expected NAME: IMPORTS or rebuild NAME")
        return

    for name, deps in imports.items():
        for dep in deps:
            if dep not in imports:
                print(f"error: line {defined_on[name]}: {name} imports {dep}, which isn't defined")
                return

    # Kahn's algorithm: a module is ready once everything it imports is built, and
    # of the ready modules, the first in alphabetical order goes next.
    users: dict[str, list[str]] = {name: [] for name in imports}
    waiting = {name: len(deps) for name, deps in imports.items()}
    for name, deps in imports.items():
        for dep in deps:
            users[dep].append(name)
    ready = [name for name, count in waiting.items() if count == 0]
    heapq.heapify(ready)
    order: list[str] = []
    while ready:
        name = heapq.heappop(ready)
        order.append(name)
        for user in users[name]:
            waiting[user] -= 1
            if waiting[user] == 0:
                heapq.heappush(ready, user)

    if len(order) < len(imports):
        # Every module left over imports another one left over, so following first
        # imports from the first module left over must come back to a module.
        left = {name for name in imports if waiting[name] > 0}
        path = [min(left)]
        seen = {path[0]: 0}
        while True:
            nxt = next(dep for dep in imports[path[-1]] if dep in left)
            if nxt in seen:
                cycle = path[seen[nxt]:] + [nxt]
                print("error: import cycle: " + " -> ".join(cycle))
                return
            seen[nxt] = len(path)
            path.append(nxt)

    print("build order: " + (", ".join(order) if order else "nothing to build"))
    position = {name: i for i, name in enumerate(order)}
    for number, name in queries:
        if name not in imports:
            print(f"error: line {number}: no module called {name}")
            continue
        affected = {name}
        stack = [name]
        while stack:
            for user in users[stack.pop()]:
                if user not in affected:
                    affected.add(user)
                    stack.append(user)
        print(f"rebuild {name}: " + ", ".join(sorted(affected, key=position.__getitem__)))


if __name__ == "__main__":
    main()
