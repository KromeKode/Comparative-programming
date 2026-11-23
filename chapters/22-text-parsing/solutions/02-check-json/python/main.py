# Reads one JSON text on stdin, strictly as RFC 8259 has it, and prints it compactly:
# no spaces, keys in the order they came, numbers as they were written, and strings
# written again with only the escapes they need. Or prints the first error's line and
# column, and exits with 1.
import sys

MAX_DEPTH = 1000
# Each level of nesting is two calls, value() and then array() or obj(), and Python
# stops at a depth of 1000 calls unless told otherwise.
sys.setrecursionlimit(4 * MAX_DEPTH)

ESCAPES = {
    '"': '"',
    "\\": "\\",
    "/": "/",
    "b": "\b",
    "f": "\f",
    "n": "\n",
    "r": "\r",
    "t": "\t",
}
SHORT = {
    '"': '\\"',
    "\\": "\\\\",
    "\b": "\\b",
    "\f": "\\f",
    "\n": "\\n",
    "\r": "\\r",
    "\t": "\\t",
}
HEX = "0123456789abcdefABCDEF"


def quote(s: str) -> str:
    """s as a JSON string, with only the escapes it needs."""
    out = ['"']
    for c in s:
        if c in SHORT:
            out.append(SHORT[c])
        elif c < " ":
            out.append(f"\\u{ord(c):04x}")
        else:
            out.append(c)
    out.append('"')
    return "".join(out)


class JsonError(Exception):
    def __init__(self, at: int, message: str) -> None:
        super().__init__(message)
        self.at = at  # an index into the text


class Parser:
    def __init__(self, text: str) -> None:
        self.text = text  # a str's indexes count code points, as columns do
        self.i = 0
        self.depth = 0

    def peek(self) -> str:
        """The character at i, or "" at the end, which is less than any character."""
        return self.text[self.i] if self.i < len(self.text) else ""

    def fail(self, expected: str) -> JsonError:
        c = self.peek()
        found = quote(c) if c else "the end of the input"
        return JsonError(self.i, f"expected {expected}, found {found}")

    def space(self) -> None:
        while self.peek() != "" and self.peek() in " \t\n\r":
            self.i += 1

    def value(self) -> str:
        c = self.peek()
        if c in ("{", "["):
            self.depth += 1
            if self.depth > MAX_DEPTH:
                raise JsonError(self.i, f"nesting deeper than {MAX_DEPTH}")
            out = self.obj() if c == "{" else self.array()
            self.depth -= 1
            return out
        if c == '"':
            return quote(self.string())
        if c == "-" or "0" <= c <= "9":
            return self.number()
        for word in ("true", "false", "null"):
            if self.text.startswith(word, self.i):
                self.i += len(word)
                return word
        raise self.fail("a value")

    def obj(self) -> str:
        self.i += 1  # the {
        self.space()
        if self.peek() == "}":
            self.i += 1
            return "{}"
        members: list[str] = []
        keys: set[str] = set()
        while True:
            if self.peek() != '"':
                raise self.fail(
                    'a string key or "}"' if not members else "a string key"
                )
            at = self.i
            key = self.string()
            if key in keys:
                raise JsonError(at, f"duplicate key {quote(key)}")
            keys.add(key)
            self.space()
            if self.peek() != ":":
                raise self.fail('":"')
            self.i += 1
            self.space()
            members.append(quote(key) + ":" + self.value())
            self.space()
            if self.peek() == "}":
                self.i += 1
                return "{" + ",".join(members) + "}"
            if self.peek() != ",":
                raise self.fail('"," or "}"')
            self.i += 1
            self.space()

    def array(self) -> str:
        self.i += 1  # the [
        self.space()
        if self.peek() == "]":
            self.i += 1
            return "[]"
        items: list[str] = []
        while True:
            items.append(self.value())
            self.space()
            if self.peek() == "]":
                self.i += 1
                return "[" + ",".join(items) + "]"
            if self.peek() != ",":
                raise self.fail('"," or "]"')
            self.i += 1
            self.space()

    def digits(self) -> None:
        if not "0" <= self.peek() <= "9":
            raise self.fail("a digit")
        while "0" <= self.peek() <= "9":
            self.i += 1

    def number(self) -> str:
        start = self.i
        if self.peek() == "-":
            self.i += 1
        if self.peek() == "0":
            self.i += 1  # a 0 can't have digits after it
        else:
            self.digits()
        if self.peek() == ".":
            self.i += 1
            self.digits()
        if self.peek() in ("e", "E"):
            self.i += 1
            if self.peek() in ("+", "-"):
                self.i += 1
            self.digits()
        return self.text[start : self.i]  # as it was written

    def hex4(self, at: int) -> int | None:
        """The four hex digits at text[at:], or None if they aren't."""
        digits = self.text[at : at + 4]
        if len(digits) < 4 or any(c not in HEX for c in digits):
            return None
        return int(digits, 16)

    def string(self) -> str:
        """The string at i, with its escapes undone."""
        start = self.i
        self.i += 1  # the "
        out: list[str] = []
        while True:
            c = self.peek()
            if c == "":
                raise JsonError(start, "string is never closed")
            if c == '"':
                self.i += 1
                return "".join(out)
            if c < " ":
                raise JsonError(self.i, "control character in a string")
            if c != "\\":
                out.append(c)
                self.i += 1
                continue
            escape = self.text[self.i + 1 : self.i + 2]
            if escape == "":
                raise JsonError(start, "string is never closed")
            if escape in ESCAPES:
                out.append(ESCAPES[escape])
                self.i += 2
                continue
            code = self.hex4(self.i + 2) if escape == "u" else None
            if code is None:
                raise JsonError(self.i, "invalid escape")
            self.i += 6
            if 0xDC00 <= code <= 0xDFFF:
                raise JsonError(self.i - 6, "unpaired surrogate")
            if 0xD800 <= code <= 0xDBFF:
                # A character above U+FFFF is two escapes, a high surrogate and a low one.
                low = (
                    self.hex4(self.i + 2)
                    if self.text.startswith("\\u", self.i)
                    else None
                )
                if low is None or not 0xDC00 <= low <= 0xDFFF:
                    raise JsonError(self.i - 6, "unpaired surrogate")
                code = 0x10000 + ((code - 0xD800) << 10) + (low - 0xDC00)
                self.i += 6
            out.append(chr(code))

    def parse(self) -> str:
        self.space()
        out = self.value()
        self.space()
        if self.i < len(self.text):
            raise self.fail("the end of the input")
        return out


def main() -> int:
    text = sys.stdin.buffer.read().decode("utf-8")
    try:
        print(Parser(text).parse())
    except JsonError as err:
        # Lines end at "\n", and columns count characters from 1.
        before = text[: err.at]
        line = before.count("\n") + 1
        column = err.at - (before.rfind("\n") + 1) + 1
        print(f"jsoncheck: line {line}, column {column}: {err}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
