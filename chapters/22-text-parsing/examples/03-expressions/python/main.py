# Reads arithmetic, one expression on each line, such as 1 + 2 * 3, and prints each with
# its parentheses written out, and its value: (1 + (2 * 3)) = 7. The numbers are whole
# and fit in 64 bits; the operators are + - * / % ^ and unary -, with ( and ). / and %
# round toward zero, as in C. An error is printed with its line and a caret under its
# column, the next line is read, and at the end, the exit status is 1.
import sys
from dataclasses import dataclass

MAX = 2**63 - 1
MIN = -(2**63)


class CalcError(Exception):
    def __init__(self, column: int, message: str) -> None:
        super().__init__(message)
        self.column = column  # counted in characters, from 1


# The lexer turns the text into tokens: numbers, and operators and parentheses, each with
# its column.
@dataclass(frozen=True)
class Token:
    kind: str  # "number", "end" after the last token, or the character: "+", "(", ...
    text: str
    column: int
    value: int = 0  # a number's value


def describe(token: Token) -> str:
    return "the end of the line" if token.kind == "end" else f'"{token.text}"'


def tokens(line: str) -> list[Token]:
    found: list[Token] = []
    i = 0
    while i < len(line):  # a str's indexes count code points, so i + 1 is the column
        char = line[i]
        if char in " \t":
            i += 1
        # Not char.isdigit(), which is true for "٣" and "²", and int() takes "٣".
        elif "0" <= char <= "9":
            start = i
            while i < len(line) and "0" <= line[i] <= "9":
                i += 1
            value = int(line[start:i])
            if value > MAX:
                raise CalcError(start + 1, "number too large for 64 bits")
            found.append(Token("number", line[start:i], start + 1, value))
        elif char in "+-*/%^()":
            found.append(Token(char, char, i + 1))
            i += 1
        else:
            raise CalcError(i + 1, f'unexpected "{char}"')
    found.append(Token("end", "", len(line) + 1))
    return found


# The parser turns the tokens into a tree, whose nodes are of three kinds.
@dataclass(frozen=True)
class Number:
    value: int


@dataclass(frozen=True)
class Negate:
    operand: "Expr"  # in quotes, because Expr is defined below, from these classes
    column: int  # where the operator is, for an error such as overflow


@dataclass(frozen=True)
class Binary:
    op: str
    left: "Expr"
    right: "Expr"
    column: int


type Expr = Number | Negate | Binary

# How tightly each operator holds the expressions on its left and right: the higher, the
# tighter. Left is less than right for left-associative operators, so 1 - 2 - 3 is
# (1 - 2) - 3, and more for ^, so 2 ^ 3 ^ 2 is 2 ^ (3 ^ 2).
INFIX = {"+": (1, 2), "-": (1, 2), "*": (3, 4), "/": (3, 4), "%": (3, 4), "^": (8, 7)}
# Unary minus holds less tightly than ^, so -2 ^ 2 is -(2 ^ 2), and more than *.
PREFIX = 5


class Parser:
    def __init__(self, line: str) -> None:
        self.tokens = tokens(line)
        self.at = 0

    def next(self) -> Token:
        token = self.tokens[self.at]
        if token.kind != "end":
            self.at += 1
        return token

    def expression(self, min_power: int) -> Expr:
        """Parses the longest expression whose operators hold at least min_power."""
        token = self.next()
        left: Expr
        match token.kind:
            case "number":
                left = Number(token.value)
            case "-":
                left = Negate(self.expression(PREFIX), token.column)
            case "(":
                left = self.expression(0)
                close = self.next()
                if close.kind != ")":
                    raise CalcError(
                        close.column, f'expected ")", found {describe(close)}'
                    )
            case _:
                message = f'expected a number, "(", or "-", found {describe(token)}'
                raise CalcError(token.column, message)
        while True:
            token = self.tokens[self.at]
            powers = INFIX.get(token.kind)
            if powers is None or powers[0] < min_power:
                return left
            self.next()
            left = Binary(token.kind, left, self.expression(powers[1]), token.column)

    def parse(self) -> Expr:
        expr = self.expression(0)
        token = self.next()
        if token.kind != "end":
            raise CalcError(
                token.column, f"expected an operator, found {describe(token)}"
            )
        return expr


def show(expr: Expr) -> str:
    match expr:
        case Number(value):
            return str(value)
        case Negate(operand):
            return f"(-{show(operand)})"
        case Binary(op, left, right):
            return f"({show(left)} {op} {show(right)})"


# Python's ints have no limit, so each result is checked against 64 bits by hand.
def fit(value: int, column: int) -> int:
    if not MIN <= value <= MAX:
        raise CalcError(column, "overflow")
    return value


def power(base: int, exponent: int, column: int) -> int:
    # base ** exponent would work out 1 ** 9223372036854775807 by multiplying, and
    # 2 ** 9223372036854775807 until memory ran out. This squares the base for each bit
    # of the exponent, and stops at the first overflow.
    result = 1
    while exponent > 0:
        if exponent % 2 == 1:
            result = fit(result * base, column)
        exponent //= 2
        if exponent > 0:
            base = fit(base * base, column)
    return result


def evaluate(expr: Expr) -> int:
    match expr:
        case Number(value):
            return value
        case Negate(operand, column):
            return fit(-evaluate(operand), column)
        case Binary(op, left, right, column):
            a, b = evaluate(left), evaluate(right)
            if op in "/%" and b == 0:
                raise CalcError(column, "division by zero")
            match op:
                case "+":
                    return fit(a + b, column)
                case "-":
                    return fit(a - b, column)
                case "*":
                    return fit(a * b, column)
                # Python's // and % round toward minus infinity, so -7 // 2 is -4 and
                # -7 % 2 is 1. These round toward zero, as C's / and % do.
                case "/":
                    quotient = abs(a) // abs(b)
                    return fit(quotient if (a < 0) == (b < 0) else -quotient, column)
                case "%":
                    remainder = abs(a) % abs(b)
                    return remainder if a >= 0 else -remainder
                case _:  # "^"
                    if b < 0:
                        raise CalcError(column, "negative exponent")
                    return power(a, b, column)


def main() -> int:
    status = 0
    text = sys.stdin.read()
    lines = text.split("\n")
    if lines[-1] == "":  # the input ended with a newline, or was empty
        lines.pop()
    for number, line in enumerate(lines, 1):
        line = line.removesuffix("\r")
        if line.strip(" \t") == "":
            continue
        try:
            expr = Parser(line).parse()
            print(f"{show(expr)} = {evaluate(expr)}")
        except CalcError as err:
            # The caret goes under the column. A tab before it stays a tab, so that the
            # caret lines up with the text whatever a tab's width is.
            pad = "".join("\t" if c == "\t" else " " for c in line[: err.column - 1])
            print(f"calc: line {number}, column {err.column}: {err}", file=sys.stderr)
            print(f"  {line}\n  {pad}^", file=sys.stderr)
            status = 1
    return status


if __name__ == "__main__":
    sys.exit(main())
