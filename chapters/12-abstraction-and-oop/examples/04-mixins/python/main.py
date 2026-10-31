class Describe:
    """A mixin: describe() works for any class that has a name."""

    name: str

    def describe(self) -> str:
        return f"this is the {self.name}"


class Logged:
    """A mixin with state: each object gets its own history."""

    name: str

    def __init__(self) -> None:
        super().__init__()
        self.history: list[str] = []

    def log(self, event: str) -> None:
        self.history.append(f"{self.name}: {event}")


# Door and Light share no base class of their own. Each one mixes in the
# behavior it wants.
class Door(Describe, Logged):
    def __init__(self, name: str) -> None:
        super().__init__()  # finds Logged.__init__ by searching the MRO
        self.name = name

    def open(self) -> None:
        self.log("opened")

    def close(self) -> None:
        self.log("closed")


class Light(Describe, Logged):
    def __init__(self, name: str) -> None:
        super().__init__()
        self.name = name
        self.on = False

    def switch_on(self) -> None:
        self.on = True
        self.log("switched on")

    # Light replaces the default describe, and reuses it.
    def describe(self) -> str:
        state = "on" if self.on else "off"
        return f"{super().describe()}, and it is {state}"


door = Door("front door")
light = Light("porch light")
door.open()
door.close()
light.switch_on()

things: list[Door | Light] = [door, light]
for thing in things:
    print(thing.describe())
    for line in thing.history:
        print(f"  {line}")
