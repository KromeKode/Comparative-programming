# Prints each line of stdin that has a match for the pattern, as grep does, with a
# matcher of its own: the pattern becomes a tree, the tree becomes a program for a
# Thompson NFA, and the NFA reads each line once, keeping the set of states it could be
# in. So a line takes time in proportion to its length times the pattern's, whatever
# the pattern is.
import sys
from dataclasses import dataclass, field

# What a "\" can make a literal: the characters that mean something, inside brackets or
# out.
SPECIAL = "\\.[]()*+?|^$-"
REPEATS = "*+?"


class PatternError(Exception):
    def __init__(self, at: int, message: str) -> None:
        super().__init__(message)
        self.at = at  # an index into the pattern's characters


# The tree. A character, a ".", and brackets are each a set of characters: "a" is the
# range a to a, and "." is the empty set, negated.
@dataclass
class Chars:
    ranges: list[tuple[str, str]]
    negated: bool = False


@dataclass
class Anchor:
    kind: str  # "^" or "$"


@dataclass
class Concat:
    items: list["Node"]


@dataclass
class Alternation:
    options: list["Node"]


@dataclass
class Repeat:
    item: "Node"
    kind: str  # "*", "+", or "?"


Node = Chars | Anchor | Concat | Alternation | Repeat


class Parser:
    """alternation = concat ("|" concat)*, concat = repeat*, repeat = atom [*+?]"""

    def __init__(self, pattern: str) -> None:
        self.pattern = pattern  # a str is code points, so an index counts characters
        self.i = 0

    def peek(self) -> str | None:
        return self.pattern[self.i] if self.i < len(self.pattern) else None

    def parse(self) -> Node:
        node = self.alternation()
        if self.i < len(self.pattern):
            # alternation() stops early only at a ")".
            raise PatternError(self.i, 'unmatched ")"')
        return node

    def alternation(self) -> Node:
        options = [self.concat()]
        while self.peek() == "|":
            self.i += 1
            options.append(self.concat())
        return options[0] if len(options) == 1 else Alternation(options)

    def concat(self) -> Node:
        items = []
        while (c := self.peek()) is not None and c not in "|)":
            items.append(self.repeat())
        return Concat(items)

    def repeat(self) -> Node:
        c = self.peek()
        # At the start, or after "(", "|", "^", "$", or another "*", "+", or "?".
        if c is not None and c in REPEATS:
            raise PatternError(self.i, "nothing to repeat")
        if c == "^" or c == "$":
            self.i += 1
            return Anchor(c)
        item = self.atom()
        c = self.peek()
        if c is not None and c in REPEATS:
            self.i += 1
            return Repeat(item, c)
        return item

    def atom(self) -> Node:
        c = self.peek()
        if c == "(":
            start = self.i
            self.i += 1
            node = self.alternation()
            if self.peek() != ")":
                raise PatternError(start, 'unclosed "("')
            self.i += 1
            return node
        if c == "[":
            return self.brackets()
        if c == ".":
            self.i += 1
            return Chars([], negated=True)
        c = self.char()
        return Chars([(c, c)])

    def char(self) -> str:
        """A character, or a "\\" and the character it makes a literal."""
        c = self.pattern[self.i]
        if c != "\\":
            self.i += 1
            return c
        if self.i + 1 == len(self.pattern):
            raise PatternError(self.i, "trailing backslash")
        c = self.pattern[self.i + 1]
        if c not in SPECIAL:
            raise PatternError(self.i, "invalid escape")
        self.i += 2
        return c

    def brackets(self) -> Node:
        start = self.i
        self.i += 1  # the [
        negated = self.peek() == "^"
        if negated:
            self.i += 1
        if self.peek() == "]":
            raise PatternError(start, "empty brackets")
        ranges = []
        while (c := self.peek()) != "]":
            if c is None:
                raise PatternError(start, 'unclosed "["')
            at = self.i
            low = self.char()
            # A "-" makes a range, unless it's last: "[a-]" is "a" and "-".
            rest = self.pattern[self.i : self.i + 2]
            if len(rest) == 2 and rest[0] == "-" and rest[1] != "]":
                self.i += 1
                high = self.char()
                if high < low:
                    raise PatternError(at, "range out of order")
                ranges.append((low, high))
            else:
                ranges.append((low, low))
        self.i += 1  # the ]
        return Chars(ranges, negated)


# The program. "chars" reads a character, if it's in the set, and goes on to the next
# instruction; "split" goes on to both x and y; "jump" goes on to x; "^" and "$" go on
# if the position is the start or the end of the line; and "match" is a match.
@dataclass
class Instruction:
    op: str
    ranges: list[tuple[str, str]] = field(default_factory=list)
    negated: bool = False
    x: int = 0
    y: int = 0


def compile_node(node: Node, program: list[Instruction]) -> None:
    match node:
        case Chars(ranges, negated):
            program.append(Instruction("chars", ranges, negated))
        case Anchor(kind):
            program.append(Instruction(kind))
        case Concat(items):
            for item in items:
                compile_node(item, program)
        case Alternation(options):
            # split L1, L2; L1: first; jump END; L2: split L3, L4; L3: second; ...
            jumps = []
            for option in options[:-1]:
                split = Instruction("split", x=len(program) + 1)
                program.append(split)
                compile_node(option, program)
                jumps.append(Instruction("jump"))
                program.append(jumps[-1])
                split.y = len(program)
            compile_node(options[-1], program)
            for jump in jumps:
                jump.x = len(program)
        case Repeat(item, "*"):
            # L1: split L2, L3; L2: item; jump L1; L3:
            start = len(program)
            split = Instruction("split", x=start + 1)
            program.append(split)
            compile_node(item, program)
            program.append(Instruction("jump", x=start))
            split.y = len(program)
        case Repeat(item, "+"):
            # L1: item; split L1, L2; L2:
            start = len(program)
            compile_node(item, program)
            program.append(Instruction("split", x=start, y=len(program) + 1))
        case Repeat(item, _):
            # split L1, L2; L1: item; L2:
            split = Instruction("split", x=len(program) + 1)
            program.append(split)
            compile_node(item, program)
            split.y = len(program)


def compile_pattern(pattern: str) -> list[Instruction]:
    program: list[Instruction] = []
    compile_node(Parser(pattern).parse(), program)
    program.append(Instruction("match"))
    return program


def follow(
    program: list[Instruction], pcs: list[int], at: int, line: str
) -> tuple[list[int], bool]:
    """The "chars" instructions that pcs lead to without reading a character, at the
    position at, and whether one of them leads to "match"."""
    waiting = []
    seen = set()
    stack = pcs[:]
    while stack:
        pc = stack.pop()
        if pc in seen:
            continue  # so that a loop such as (a*)* doesn't go around forever
        seen.add(pc)
        instruction = program[pc]
        match instruction.op:
            case "chars":
                waiting.append(pc)
            case "split":
                stack += [instruction.y, instruction.x]
            case "jump":
                stack.append(instruction.x)
            case "^" if at == 0:
                stack.append(pc + 1)
            case "$" if at == len(line):
                stack.append(pc + 1)
            case "match":
                return [], True
    return waiting, False


def accepts(instruction: Instruction, c: str) -> bool:
    inside = any(low <= c <= high for low, high in instruction.ranges)
    return inside != instruction.negated


def search(program: list[Instruction], line: str) -> bool:
    """Whether a match starts anywhere in the line: program 0 starts at each position."""
    waiting: list[int] = []
    for at in range(len(line) + 1):
        reached, matched = follow(program, waiting + [0], at, line)
        if matched:
            return True
        if at < len(line):
            waiting = [pc + 1 for pc in reached if accepts(program[pc], line[at])]
    return False


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: rematch PATTERN", file=sys.stderr)
        return 2
    try:
        program = compile_pattern(sys.argv[1])
    except PatternError as err:
        print(f"rematch: column {err.at + 1}: {err}", file=sys.stderr)
        return 2
    lines = sys.stdin.buffer.read().decode("utf-8").split("\n")
    if lines[-1] == "":
        lines.pop()  # what comes after the last "\n", or the empty input
    found = [line for line in lines if search(program, line)]
    sys.stdout.write("".join(line + "\n" for line in found))
    return 0 if found else 1


if __name__ == "__main__":
    sys.exit(main())
