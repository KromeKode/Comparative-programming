from typing import Protocol


# A Protocol is an interface that a class meets by having the methods, without
# naming the Protocol. mypy checks that whatever is passed in has them.
class Sensor(Protocol):
    def read_celsius(self) -> int: ...


class Heater(Protocol):
    def turn_on(self) -> None: ...

    def turn_off(self) -> None: ...


def regulate(sensor: Sensor, heater: Heater, target: int) -> None:
    """Turns the heater on if the room is more than a degree below target, and off
    if it is more than a degree above. Within a degree, it leaves the heater alone."""
    celsius = sensor.read_celsius()
    if celsius < target - 1:
        heater.turn_on()
    elif celsius > target + 1:
        heater.turn_off()
