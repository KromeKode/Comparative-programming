import { readFileSync } from "node:fs";

// The value parsers throw ValueProblem; load wraps it in a BadValue.
class ValueProblem extends Error {}

// The base class for every problem load can find in a file.
class ConfigError extends Error {}

class SyntaxProblem extends ConfigError {
  constructor(line) {
    super(`line ${line}: expected "key = value"`);
  }
}

class UnknownKey extends ConfigError {
  constructor(line, key) {
    super(`line ${line}: unknown key "${key}"`);
  }
}

class DuplicateKey extends ConfigError {
  constructor(line, key, first) {
    super(`line ${line}: "${key}" is already set on line ${first}`);
  }
}

class BadValue extends ConfigError {
  constructor(line, key, problem) {
    super(`line ${line}: ${key}: ${problem.message}`, { cause: problem });
  }
}

class MissingKey extends ConfigError {
  constructor(key) {
    super(`missing key "${key}"`);
  }
}

const KINDS = [
  ["syntax", SyntaxProblem],
  ["unknown key", UnknownKey],
  ["duplicate key", DuplicateKey],
  ["bad value", BadValue],
  ["missing key", MissingKey],
];

function parseText(text) {
  if (text === "") {
    throw new ValueProblem("must not be empty");
  }
  return text;
}

function parseWhole(text, low, high) {
  if (!/^[0-9]+$/.test(text)) {
    throw new ValueProblem(`"${text}" is not a whole number`);
  }
  let value = 0;
  for (const digit of text) {
    // Stop growing past the limit, so a long number stays exact.
    value = Math.min(value * 10 + Number(digit), high + 1);
  }
  if (value < low || value > high) {
    throw new ValueProblem(`${text} is not between ${low} and ${high}`);
  }
  return value;
}

function parseBool(text) {
  if (text !== "true" && text !== "false") {
    throw new ValueProblem(`"${text}" is not true or false`);
  }
  return text === "true";
}

const PARSERS = new Map([
  ["name", parseText],
  ["port", (text) => parseWhole(text, 1, 65535)],
  ["debug", parseBool],
  ["timeout", (text) => parseWhole(text, 1, 3600)],
]);

// Return the config, or throw an AggregateError that holds every problem.
function load(lines) {
  const config = { debug: false, timeout: 30 };
  const firstLine = new Map();
  const errors = [];
  lines.forEach((raw, index) => {
    const number = index + 1;
    const line = raw.trim();
    if (line === "" || line.startsWith("#")) {
      return;
    }
    const equals = line.indexOf("=");
    const key = line.slice(0, equals).trim();
    if (equals === -1 || key === "") {
      errors.push(new SyntaxProblem(number));
    } else if (!PARSERS.has(key)) {
      errors.push(new UnknownKey(number, key));
    } else if (firstLine.has(key)) {
      errors.push(new DuplicateKey(number, key, firstLine.get(key)));
    } else {
      firstLine.set(key, number);
      try {
        config[key] = PARSERS.get(key)(line.slice(equals + 1).trim());
      } catch (problem) {
        if (!(problem instanceof ValueProblem)) {
          throw problem;
        }
        errors.push(new BadValue(number, key, problem));
      }
    }
  });
  for (const key of ["name", "port"]) {
    if (!firstLine.has(key)) {
      errors.push(new MissingKey(key));
    }
  }
  if (errors.length > 0) {
    throw new AggregateError(errors, "the config has errors");
  }
  return config;
}

const files = [];
for (const line of readFileSync(0, "utf8").split("\n")) {
  if (line.startsWith("== ")) {
    files.push({ name: line.slice(3), lines: [] });
  } else if (files.length > 0) {
    files.at(-1).lines.push(line);
  }
}
// split leaves an empty string after the last newline.
if (files.length > 0 && files.at(-1).lines.at(-1) === "") {
  files.at(-1).lines.pop();
}

const counts = KINDS.map(() => 0);
let loaded = 0;
for (const { name, lines } of files) {
  let config;
  try {
    config = load(lines);
  } catch (error) {
    if (!(error instanceof AggregateError)) {
      throw error;
    }
    const count = error.errors.length;
    console.log(`${name}: ${count} error${count === 1 ? "" : "s"}`);
    for (const problem of error.errors) {
      console.log(`  ${problem.message}`);
      counts[KINDS.findIndex(([, type]) => problem instanceof type)]++;
    }
    continue;
  }
  loaded++;
  console.log(`${name}: ok`);
  for (const key of ["name", "port", "debug", "timeout"]) {
    console.log(`  ${key} = ${config[key]}`);
  }
}
console.log(`files loaded: ${loaded} of ${files.length}`);
console.log(KINDS.map(([kind], i) => `${kind}: ${counts[i]}`).join(", "));
