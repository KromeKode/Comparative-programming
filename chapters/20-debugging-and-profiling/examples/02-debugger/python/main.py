# Evaluates arithmetic with +, -, *, and parentheses: each argument, or each line of
# input if there are none. Section 20.5 runs it in a debugger, to watch the parser
# call itself. It expects well-formed expressions, and doesn't check.
import sys


class Parser:
    """A recursive descent parser, with one method for each rule of the grammar."""

    def __init__(self, text: str) -> None:
        self.text = text
        self.pos = 0

    def peek(self) -> str:
        """Skips spaces, and gives the next character, or "" at the end."""
        while self.pos < len(self.text) and self.text[self.pos] == " ":
            self.pos += 1
        return self.text[self.pos] if self.pos < len(self.text) else ""

    # expr = term { ("+" | "-") term }
    def expr(self) -> int:
        value = self.term()
        while self.peek() in ("+", "-"):
            op = self.text[self.pos]
            self.pos += 1
            right = self.term()
            value = value + right if op == "+" else value - right
        return value

    # term = factor { "*" factor }
    def term(self) -> int:
        value = self.factor()
        while self.peek() == "*":
            self.pos += 1
            value *= self.factor()
        return value

    # factor = number | "(" expr ")"
    def factor(self) -> int:
        if self.peek() == "(":
            self.pos += 1
            value = self.expr()
            self.peek()
            self.pos += 1  # the ")"
            return value
        start = self.pos
        while self.pos < len(self.text) and self.text[self.pos].isdigit():
            self.pos += 1
        return int(self.text[start : self.pos])


def evaluate(text: str) -> int:
    return Parser(text).expr()


def main() -> None:
    for text in sys.argv[1:] or sys.stdin.read().splitlines():
        print(evaluate(text))


if __name__ == "__main__":
    main()
