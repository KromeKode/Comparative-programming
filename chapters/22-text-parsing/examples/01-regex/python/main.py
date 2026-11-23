# Reads a web server's log, one request on each line, such as
#   2026-11-23 14:05:12 GET /books/42 200 12ms
# and prints, for each method and route, how many requests there were, how many failed,
# and the slowest. A route is the path with each "/" and the digits after it written
# "/:id", so that /books/42 and /books/7 count together. A line that isn't a request
# gets a message on stderr, and the exit status is then 1.
import re
import sys
from dataclasses import dataclass

# [0-9], not \d: in Python, \d matches any Unicode digit, such as the Arabic-Indic ٢.
# The time has at most nine digits, so that it fits in 32 bits in every language.
LINE = re.compile(
    r"(?P<date>[0-9]{4}-[0-9]{2}-[0-9]{2}) (?P<time>[0-9]{2}:[0-9]{2}:[0-9]{2})"
    r" (?P<method>GET|POST|PUT|DELETE) (?P<path>/[^ ]*)"
    r" (?P<status>[0-9]{3}) (?P<ms>[0-9]{1,9})ms"
)
ID = re.compile(r"/[0-9]+")


@dataclass
class Route:
    requests: int = 0
    failed: int = 0
    slowest: int = 0


def main() -> int:
    routes: dict[str, Route] = {}  # a dict keeps its keys in the order they went in
    status = 0
    # On Linux and macOS, sys.stdin keeps a "\r" before each "\n". splitlines() would
    # also split at characters such as U+2028, so this splits at "\n" only.
    lines = sys.stdin.read().split("\n")
    if lines[-1] == "":  # the input ended with a newline, or was empty
        lines.pop()
    for number, line in enumerate(lines, 1):
        line = line.removesuffix("\r")
        # fullmatch: the whole line, not just its start, as match would.
        match = LINE.fullmatch(line)
        if match is None:
            print(f"logscan: line {number}: not a log line", file=sys.stderr)
            status = 1
            continue
        key = match["method"] + " " + ID.sub("/:id", match["path"])
        route = routes.setdefault(key, Route())
        route.requests += 1
        route.failed += int(match["status"]) >= 400
        route.slowest = max(route.slowest, int(match["ms"]))
    for key, route in routes.items():
        plural = "" if route.requests == 1 else "s"
        print(
            f"{key}: {route.requests} request{plural}, {route.failed} failed,"
            f" slowest {route.slowest}ms"
        )
    return status


if __name__ == "__main__":
    sys.exit(main())
