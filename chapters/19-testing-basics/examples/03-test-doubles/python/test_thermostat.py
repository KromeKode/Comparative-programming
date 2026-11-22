from unittest.mock import Mock

from thermostat import Heater, Sensor, regulate


class StubSensor:
    """A stub: a sensor that reads whatever temperature the test gives it."""

    def __init__(self, celsius: int) -> None:
        self.celsius = celsius

    def read_celsius(self) -> int:
        return self.celsius


class SpyHeater:
    """A spy: a heater that records the calls made to it, for the test to check."""

    def __init__(self) -> None:
        self.calls: list[str] = []

    def turn_on(self) -> None:
        self.calls.append("on")

    def turn_off(self) -> None:
        self.calls.append("off")


def test_too_cold() -> None:
    heater = SpyHeater()
    regulate(StubSensor(17), heater, 20)
    assert heater.calls == ["on"]


def test_too_hot() -> None:
    heater = SpyHeater()
    regulate(StubSensor(23), heater, 20)
    assert heater.calls == ["off"]


def test_close_enough() -> None:
    for celsius in [19, 20, 21]:
        heater = SpyHeater()
        regulate(StubSensor(celsius), heater, 20)
        assert heater.calls == [], f"at {celsius}"


# The same test as test_too_hot, with the standard library's Mock instead: a mock
# records each call, and returns what it was told to. With a spec, it accepts only
# the methods that the class it stands in for has.
def test_too_hot_with_mocks() -> None:
    sensor = Mock(spec=Sensor)
    sensor.read_celsius.return_value = 23
    heater = Mock(spec=Heater)
    regulate(sensor, heater, 20)
    heater.turn_off.assert_called_once_with()
    heater.turn_on.assert_not_called()
