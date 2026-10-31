from enum import StrEnum, auto


# On a StrEnum, auto() makes each value the member's name in lowercase.
class Light(StrEnum):
    RED = auto()
    GREEN = auto()
    YELLOW = auto()

    def next(self) -> "Light":
        members = list(Light)
        return members[(members.index(self) + 1) % len(members)]

    def meaning(self) -> str:
        match self:
            case Light.RED:
                return "stop"
            case Light.GREEN:
                return "go"
            case Light.YELLOW:
                return "slow down"


SECONDS = {Light.RED: 30, Light.GREEN: 25, Light.YELLOW: 5}


def parse_light(text: str) -> Light | None:
    try:
        return Light(text)
    except ValueError:
        return None


def show_parse(text: str) -> None:
    light = parse_light(text)
    print(f'parse "{text}": {light if light is not None else "not a light"}')


def main() -> None:
    print("lights:", " ".join(Light))
    print("yellow is number", list(Light).index(Light.YELLOW))

    light = Light.RED
    for _ in Light:
        print(f"after {light}: {light.next()}")
        light = light.next()
    print(
        f"red means {Light.RED.meaning()}, green means {Light.GREEN.meaning()}, "
        f"yellow means {Light.YELLOW.meaning()}"
    )

    print(f"one cycle: {sum(SECONDS[light] for light in Light)} seconds")

    show_parse("green")
    show_parse("blue")


if __name__ == "__main__":
    main()
