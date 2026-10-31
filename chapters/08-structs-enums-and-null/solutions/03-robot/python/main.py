import sys
from dataclasses import dataclass
from enum import Enum


# The four directions, in clockwise order, so turning right moves one step along.
class Direction(Enum):
    NORTH = (0, 1)
    EAST = (1, 0)
    SOUTH = (0, -1)
    WEST = (-1, 0)

    def turn(self, steps: int) -> "Direction":
        order = list(Direction)
        return order[(order.index(self) + steps) % len(order)]


@dataclass
class Robot:
    x: int = 0
    y: int = 0
    facing: Direction = Direction.NORTH

    def run(self, command: str) -> None:
        match command:
            case "L":
                self.facing = self.facing.turn(-1)
            case "R":
                self.facing = self.facing.turn(1)
            case "F":
                dx, dy = self.facing.value
                self.x += dx
                self.y += dy


def main() -> None:
    for line in sys.stdin:
        program = line.rstrip("\n")
        if program == "":
            continue
        bad = next(((i, c) for i, c in enumerate(program, 1) if c not in "LRF"), None)
        if bad is not None:
            position, command = bad
            print(f"error: unknown command '{command}' at position {position}")
            continue
        robot = Robot()
        for command in program:
            robot.run(command)
        print(robot.x, robot.y, robot.facing.name.lower())


if __name__ == "__main__":
    main()
