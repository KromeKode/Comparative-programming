# Converts temperatures, such as 21.5c, -40f, or 300k, to Celsius, Fahrenheit, or
# Kelvin. argparse reads the options, prints --help, and exits with status 2, after a
# message on stderr, when the command line is wrong.
import argparse
import math
import sys
from collections.abc import Callable

TO_CELSIUS: dict[str, Callable[[float], float]] = {
    "c": lambda t: t,
    "f": lambda t: (t - 32) * 5 / 9,
    "k": lambda t: t - 273.15,
}
FROM_CELSIUS: dict[str, Callable[[float], float]] = {
    "c": lambda t: t,
    "f": lambda t: t * 9 / 5 + 32,
    "k": lambda t: t + 273.15,
}


def parse_temperature(text: str) -> tuple[float, str] | None:
    """A number followed by c, f, or k, or None."""
    unit = text[-1:]
    if unit not in TO_CELSIUS:
        return None
    try:
        value = float(text[:-1])
    except ValueError:
        return None
    return (value, unit) if math.isfinite(value) else None


def main() -> int:
    parser = argparse.ArgumentParser(
        prog="temp", description="Converts temperatures, such as 21.5c, -40f, or 300k."
    )
    parser.add_argument(
        "-t",
        "--to",
        choices=["c", "f", "k"],
        default="c",
        help="the unit to convert to (default: c)",
    )
    parser.add_argument(
        "-d",
        "--digits",
        type=int,
        choices=range(10),
        default=1,
        metavar="DIGITS",
        help="digits after the point, 0 to 9 (default: 1)",
    )
    parser.add_argument("temperatures", nargs="+", metavar="TEMPERATURE")
    args = parser.parse_args()  # exits with status 2 if the arguments are wrong

    # Check every temperature before printing any, so that a mistake prints nothing.
    temperatures = []
    for text in args.temperatures:
        parsed = parse_temperature(text)
        if parsed is None:
            print(f"temp: not a temperature: {text}", file=sys.stderr)
            return 2
        temperatures.append((text, *parsed))

    for text, value, unit in temperatures:
        converted = FROM_CELSIUS[args.to](TO_CELSIUS[unit](value))
        print(f"{text} = {converted:.{args.digits}f}{args.to}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
