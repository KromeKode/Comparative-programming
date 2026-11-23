// Converts temperatures, such as 21.5c, -40f, or 300k, to Celsius, Fahrenheit, or
// Kelvin. util.parseArgs, built into Node, reads the options, and throws when the
// command line is wrong; the usage message and the checks of each value are ours.
import { parseArgs } from "node:util";

const USAGE = "usage: temp [-t UNIT] [-d DIGITS] TEMPERATURE...";
const HELP = `${USAGE}

Converts temperatures, such as 21.5c, -40f, or 300k.

options:
  -h, --help           show this help and exit
  -t, --to UNIT        the unit to convert to: c, f, or k (default: c)
  -d, --digits DIGITS  digits after the point, 0 to 9 (default: 1)`;

const TO_CELSIUS = {
  c: (t) => t,
  f: (t) => ((t - 32) * 5) / 9,
  k: (t) => t - 273.15,
};
const FROM_CELSIUS = {
  c: (t) => t,
  f: (t) => (t * 9) / 5 + 32,
  k: (t) => t + 273.15,
};

// Prints a message and the usage line to stderr, and exits with status 2.
function usageError(message) {
  console.error(`temp: ${message}\n${USAGE}`);
  process.exit(2);
}

// A number followed by c, f, or k, or null.
function parseTemperature(text) {
  const unit = text.slice(-1);
  const number = text.slice(0, -1);
  // Number("") is 0, so an empty number needs a check of its own.
  if (!Object.hasOwn(TO_CELSIUS, unit) || number.trim() === "") return null;
  const value = Number(number);
  return Number.isFinite(value) ? { value, unit } : null;
}

let parsed;
try {
  parsed = parseArgs({
    options: {
      help: { type: "boolean", short: "h" },
      to: { type: "string", short: "t", default: "c" },
      digits: { type: "string", short: "d", default: "1" },
    },
    allowPositionals: true,
  });
} catch (error) {
  usageError(error.message);
}
const { values, positionals } = parsed;
if (values.help) {
  console.log(HELP);
  process.exit(0);
}
const to = values.to;
if (!Object.hasOwn(FROM_CELSIUS, to)) usageError(`unknown unit: ${to}`);
const digits = Number.parseInt(values.digits, 10);
// String(digits) is values.digits only when it was a plain whole number, such as 3.
if (String(digits) !== values.digits || digits > 9) {
  usageError(`digits must be 0 to 9, not ${values.digits}`);
}
if (positionals.length === 0) usageError("no temperatures");

// Check every temperature before printing any, so that a mistake prints nothing.
const temperatures = positionals.map((text) => {
  const temperature = parseTemperature(text);
  if (temperature === null) {
    console.error(`temp: not a temperature: ${text}`);
    process.exit(2);
  }
  return { text, ...temperature };
});

for (const { text, value, unit } of temperatures) {
  const converted = FROM_CELSIUS[to](TO_CELSIUS[unit](value));
  console.log(`${text} = ${converted.toFixed(digits)}${to}`);
}
