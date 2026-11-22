import string
import sys
from dataclasses import dataclass

WORD_CHARS = frozenset(string.ascii_letters + string.digits + "_")


@dataclass
class Macro:
    params: list[str] | None  # None for a macro that takes no parentheses
    body: str


class ExpandError(Exception):
    pass


macros: dict[str, Macro] = {}


def is_name(word: str) -> bool:
    return word != "" and not word[0].isdigit() and all(c in WORD_CHARS for c in word)


def word_end(text: str, start: int) -> int:
    end = start
    while end < len(text) and text[end] in WORD_CHARS:
        end += 1
    return end


def string_end(text: str, start: int) -> int:
    """The index just past the string that starts at text[start], a quote."""
    close = text.find('"', start + 1)
    return len(text) if close < 0 else close + 1


def split_args(text: str, open_paren: int, name: str) -> tuple[list[str], int]:
    """Splits the arguments of a call at its top-level commas."""
    args: list[str] = []
    depth = 0
    start = i = open_paren + 1
    while i < len(text):
        c = text[i]
        if c == '"':
            i = string_end(text, i)
            continue
        if c == "(":
            depth += 1
        elif c == ")" and depth > 0:
            depth -= 1
        elif c == ")":
            args.append(text[start:i])
            return args, i + 1
        elif c == "," and depth == 0:
            args.append(text[start:i])
            start = i + 1
        i += 1
    raise ExpandError(f"unterminated call to {name}")


def substitute(macro: Macro, args: list[str], disabled: frozenset[str]) -> str:
    """The macro's body, with each parameter replaced by its argument."""
    assert macro.params is not None
    raw = dict(zip(macro.params, args))
    expanded: dict[str, str] = {}  # an argument is expanded only if it is used
    out: list[str] = []
    body = macro.body
    i = 0
    while i < len(body):
        c = body[i]
        if c == '"':
            end = string_end(body, i)
        elif c == "#":
            # # and a parameter make a string of the argument as written.
            start = i + 1
            while start < len(body) and body[start] == " ":
                start += 1
            end = word_end(body, start)
            if body[start:end] in raw:
                out.append(f'"{raw[body[start:end]]}"')
                i = end
                continue
            end = i + 1
        elif c in WORD_CHARS:
            end = word_end(body, i)
            word = body[i:end]
            if word in raw:
                if word not in expanded:
                    expanded[word] = expand(raw[word], disabled)
                out.append(expanded[word])
                i = end
                continue
        else:
            end = i + 1
        out.append(body[i:end])
        i = end
    return "".join(out)


def expand(text: str, disabled: frozenset[str]) -> str:
    """Expands every macro in text, except those in disabled."""
    out: list[str] = []
    i = 0
    while i < len(text):
        c = text[i]
        if c == '"':
            end = string_end(text, i)
        elif c in WORD_CHARS:
            end = word_end(text, i)
            word = text[i:end]
            macro = macros.get(word)
            if macro is not None and word not in disabled:
                # Inside its own expansion, a macro's name is left alone.
                inner = disabled | {word}
                if macro.params is None:
                    out.append(expand(macro.body, inner))
                    i = end
                    continue
                paren = end
                while paren < len(text) and text[paren] == " ":
                    paren += 1
                if paren < len(text) and text[paren] == "(":
                    args, i = split_args(text, paren, word)
                    args = [arg.strip() for arg in args]
                    if macro.params == [] and args == [""]:
                        args = []
                    if len(args) != len(macro.params):
                        count = len(macro.params)
                        noun = "argument" if count == 1 else "arguments"
                        raise ExpandError(
                            f"{word} takes {count} {noun}, got {len(args)}"
                        )
                    out.append(expand(substitute(macro, args, disabled), inner))
                    continue
        else:
            end = i + 1
        out.append(text[i:end])
        i = end
    return "".join(out)


def define(rest: str) -> None:
    end = word_end(rest, 0)
    name = rest[:end]
    if not is_name(name):
        raise ExpandError("bad #define")
    if end < len(rest) and rest[end] == "(":
        close = rest.find(")", end)
        if close < 0:
            raise ExpandError("bad #define")
        inside = rest[end + 1 : close]
        params = (
            [param.strip() for param in inside.split(",")] if inside.strip() else []
        )
        if not all(is_name(param) for param in params) or len(set(params)) != len(
            params
        ):
            raise ExpandError("bad #define")
        macros[name] = Macro(params, rest[close + 1 :].strip())
    else:
        macros[name] = Macro(None, rest[end:].strip())


def run(line: str) -> str | None:
    """Returns the line to print, or None for a directive."""
    directive, _, rest = line.strip().partition(" ")
    rest = rest.strip()
    if directive == "#define":
        define(rest)
        return None
    if directive == "#undef":
        if not is_name(rest):
            raise ExpandError("bad #undef")
        macros.pop(rest, None)
        return None
    return expand(line, frozenset())


def main() -> None:
    for line in sys.stdin.read().splitlines():
        try:
            result = run(line)
        except ExpandError as error:
            result = f"error: {error}"
        if result is not None:
            print(result)


if __name__ == "__main__":
    main()
