import sys
from abc import ABC, abstractmethod
from collections.abc import Callable
from dataclasses import dataclass


class PluginError(Exception):
    pass


class Plugin(ABC):
    """Every plugin has a name and changes a list of words."""

    name: str

    @abstractmethod
    def apply(self, words: list[str]) -> list[str]: ...

    def describe(self) -> str:
        return self.name  # the default, for plugins with no settings


class Upper(Plugin):
    name = "upper"

    def apply(self, words: list[str]) -> list[str]:
        return [word.upper() for word in words]


class Lower(Plugin):
    name = "lower"

    def apply(self, words: list[str]) -> list[str]:
        return [word.lower() for word in words]


class Reverse(Plugin):
    name = "reverse"

    def apply(self, words: list[str]) -> list[str]:
        return words[::-1]


class Censor(Plugin):
    name = "censor"

    def __init__(self, word: str) -> None:
        self.word = word

    def apply(self, words: list[str]) -> list[str]:
        hidden = self.word.lower()
        return ["*" * len(word) if word.lower() == hidden else word for word in words]

    def describe(self) -> str:
        return f"censor({self.word})"


Make = Callable[[list[str]], Plugin]


@dataclass
class Entry:
    usage: str
    help: str
    make: Make


class Registry:
    """Maps a plugin's name to a way to make one. The rest of the program
    finds plugins only through here."""

    def __init__(self) -> None:
        self._entries: dict[str, Entry] = {}

    def register(self, name: str, entry: Entry) -> None:
        self._entries[name] = entry

    def create(self, name: str, args: list[str]) -> Plugin:
        if name not in self._entries:
            raise PluginError(f"no plugin named {name}")
        return self._entries[name].make(args)

    def entries(self) -> list[Entry]:
        return list(self._entries.values())


def no_arguments(plugin: type[Plugin]) -> Make:
    def make(args: list[str]) -> Plugin:
        if args:
            raise PluginError(f"{plugin.name} takes no arguments")
        return plugin()

    return make


def make_censor(args: list[str]) -> Plugin:
    if len(args) != 1:
        raise PluginError("censor needs exactly one word")
    return Censor(args[0])


def make_registry() -> Registry:
    registry = Registry()
    simple: list[tuple[type[Plugin], str]] = [
        (Upper, "make every letter uppercase"),
        (Lower, "make every letter lowercase"),
        (Reverse, "reverse the order of the words"),
    ]
    for plugin, text in simple:
        registry.register(plugin.name, Entry(plugin.name, text, no_arguments(plugin)))
    censor = Entry("censor WORD", "hide WORD wherever it appears", make_censor)
    registry.register("censor", censor)
    return registry


def main() -> None:
    registry = make_registry()
    pipeline: list[Plugin] = []
    for line in sys.stdin:
        match line.split():
            case []:
                continue
            case ["list"]:
                for entry in registry.entries():
                    print(f"{entry.usage}: {entry.help}")
            case ["add", name, *args]:
                try:
                    plugin = registry.create(name, args)
                except PluginError as error:
                    print(f"error: {error}")
                    continue
                pipeline.append(plugin)
                print(f"added {plugin.describe()}")
            case ["pipeline"]:
                steps = " | ".join(plugin.describe() for plugin in pipeline)
                print(f"pipeline: {steps or '(empty)'}")
            case ["run", *words]:
                for plugin in pipeline:
                    words = plugin.apply(words)
                print(" ".join(words))


if __name__ == "__main__":
    main()
