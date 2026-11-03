import re
import sys
from collections.abc import Callable
from typing import Protocol


class SettingError(Exception):
    pass


class Setting[T]:
    """One setting whose value is a T. It knows how to read a T from text and how to show one."""

    def __init__(
        self,
        name: str,
        default: T,
        parse: Callable[[str], T | None],
        show: Callable[[T], str],
        needs: str,
    ) -> None:
        self.name = name
        self.default = default
        self.value = default
        self._parse = parse
        self._show = show
        self._needs = needs

    def set_text(self, text: str) -> None:
        value = self._parse(text)
        if value is None:
            raise SettingError(f'{self.name} needs {self._needs}, not "{text}"')
        self.value = value

    def reset(self) -> None:
        self.value = self.default

    def describe(self) -> str:
        return f"{self.name} = {self._show(self.value)}"

    def is_default(self) -> bool:
        return self.value == self.default


# What the command loop needs from a setting, without knowing its T. Every
# Setting[T] matches, so one list can hold settings of different types.
class AnySetting(Protocol):
    name: str

    def set_text(self, text: str) -> None: ...

    def reset(self) -> None: ...

    def describe(self) -> str: ...

    def is_default(self) -> bool: ...


def parse_int(text: str) -> int | None:
    if not re.fullmatch(r"-?[0-9]+", text):
        return None
    value = int(text)
    return value if -(2**31) <= value < 2**31 else None


def parse_bool(text: str) -> bool | None:
    return {"true": True, "false": False}.get(text)


def show_tags(tags: list[str]) -> str:
    return "[" + ", ".join(tags) + "]"


class Settings:
    def __init__(self) -> None:
        # Typed handles: self.volume.value is an int, self.muted.value a bool.
        self.volume = Setting("volume", 5, parse_int, str, "a whole number")
        self.muted = Setting(
            "muted", False, parse_bool, lambda b: str(b).lower(), "true or false"
        )
        self.name = Setting("name", "guest", lambda text: text, lambda text: text, "")
        self.tags: Setting[list[str]] = Setting("tags", [], str.split, show_tags, "")
        self.all: list[AnySetting] = [self.volume, self.muted, self.name, self.tags]

    def find(self, name: str) -> AnySetting:
        for setting in self.all:
            if setting.name == name:
                return setting
        raise SettingError(f'no setting named "{name}"')

    def summary(self) -> str:
        if self.muted.value:
            return f"{self.name.value} has muted the sound"
        return f"{self.name.value} hears the sound at {self.volume.value * 10}%"


def main() -> None:
    settings = Settings()
    for line in sys.stdin:
        words = line.split()
        if not words:
            continue
        command = words[0]
        try:
            if command == "set":
                setting = settings.find(words[1])
                setting.set_text(" ".join(words[2:]))
                print(setting.describe())
            elif command == "get":
                print(settings.find(words[1]).describe())
            elif command == "reset":
                setting = settings.find(words[1])
                setting.reset()
                print(setting.describe())
            elif command == "list":
                for setting in settings.all:
                    print(
                        setting.describe()
                        + (" (default)" if setting.is_default() else "")
                    )
            elif command == "summary":
                print(settings.summary())
        except SettingError as error:
            print(f"error: {error}")


if __name__ == "__main__":
    main()
