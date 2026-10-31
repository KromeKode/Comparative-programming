import { readFileSync } from "node:fs";

interface Config {
  name: string;
  port: number;
  debug: boolean;
  timeout: number;
}

// Every problem load can find, as a union of tagged objects.
type ConfigError =
  | { kind: "syntax"; line: number }
  | { kind: "unknown key"; line: number; key: string }
  | { kind: "duplicate key"; line: number; key: string; first: number }
  | { kind: "bad value"; line: number; key: string; problem: string }
  | { kind: "missing key"; key: string };

const KINDS: ConfigError["kind"][] = [
  "syntax",
  "unknown key",
  "duplicate key",
  "bad value",
  "missing key",
];

function describe(error: ConfigError): string {
  switch (error.kind) {
    case "syntax":
      return `line ${error.line}: expected "key = value"`;
    case "unknown key":
      return `line ${error.line}: unknown key "${error.key}"`;
    case "duplicate key":
      return `line ${error.line}: "${error.key}" is already set on line ${error.first}`;
    case "bad value":
      return `line ${error.line}: ${error.key}: ${error.problem}`;
    case "missing key":
      return `missing key "${error.key}"`;
  }
}

// A value parser returns the value, or a string that says what is wrong.
type Parsed<T> = { ok: true; value: T } | { ok: false; problem: string };

function parseText(text: string): Parsed<string> {
  return text === ""
    ? { ok: false, problem: "must not be empty" }
    : { ok: true, value: text };
}

function parseWhole(text: string, low: number, high: number): Parsed<number> {
  if (!/^[0-9]+$/.test(text)) {
    return { ok: false, problem: `"${text}" is not a whole number` };
  }
  let value = 0;
  for (const digit of text) {
    // Stop growing past the limit, so a long number stays exact.
    value = Math.min(value * 10 + Number(digit), high + 1);
  }
  if (value < low || value > high) {
    return { ok: false, problem: `${text} is not between ${low} and ${high}` };
  }
  return { ok: true, value };
}

function parseBool(text: string): Parsed<boolean> {
  if (text !== "true" && text !== "false") {
    return { ok: false, problem: `"${text}" is not true or false` };
  }
  return { ok: true, value: text === "true" };
}

// Pass a parsed value to set, or return the problem if there is one.
function store<T>(parsed: Parsed<T>, set: (value: T) => void): string | null {
  if (!parsed.ok) {
    return parsed.problem;
  }
  set(parsed.value);
  return null;
}

type LoadResult =
  | { ok: true; config: Config }
  | { ok: false; errors: ConfigError[] };

function load(lines: string[]): LoadResult {
  const config: Config = { name: "", port: 0, debug: false, timeout: 30 };
  const firstLine = new Map<string, number>();
  const errors: ConfigError[] = [];
  lines.forEach((raw, index) => {
    const line = index + 1;
    const trimmed = raw.trim();
    if (trimmed === "" || trimmed.startsWith("#")) {
      return;
    }
    const equals = trimmed.indexOf("=");
    const key = trimmed.slice(0, equals).trim();
    const text = trimmed.slice(equals + 1).trim();
    if (equals === -1 || key === "") {
      errors.push({ kind: "syntax", line });
      return;
    }
    if (!["name", "port", "debug", "timeout"].includes(key)) {
      errors.push({ kind: "unknown key", line, key });
      return;
    }
    const first = firstLine.get(key);
    if (first !== undefined) {
      errors.push({ kind: "duplicate key", line, key, first });
      return;
    }
    firstLine.set(key, line);
    let problem: string | null;
    switch (key) {
      case "name":
        problem = store(parseText(text), (value) => (config.name = value));
        break;
      case "port":
        problem = store(
          parseWhole(text, 1, 65535),
          (value) => (config.port = value),
        );
        break;
      case "debug":
        problem = store(parseBool(text), (value) => (config.debug = value));
        break;
      default:
        problem = store(
          parseWhole(text, 1, 3600),
          (value) => (config.timeout = value),
        );
    }
    if (problem !== null) {
      errors.push({ kind: "bad value", line, key, problem });
    }
  });
  for (const key of ["name", "port"]) {
    if (!firstLine.has(key)) {
      errors.push({ kind: "missing key", key });
    }
  }
  return errors.length > 0 ? { ok: false, errors } : { ok: true, config };
}

const files: { name: string; lines: string[] }[] = [];
for (const line of readFileSync(0, "utf8").split("\n")) {
  if (line.startsWith("== ")) {
    files.push({ name: line.slice(3), lines: [] });
  } else {
    files.at(-1)?.lines.push(line);
  }
}
// split leaves an empty string after the last newline.
if (files.at(-1)?.lines.at(-1) === "") {
  files.at(-1)?.lines.pop();
}

const counts = new Map(KINDS.map((kind) => [kind, 0]));
let loaded = 0;
for (const { name, lines } of files) {
  const result = load(lines);
  if (!result.ok) {
    const count = result.errors.length;
    console.log(`${name}: ${count} error${count === 1 ? "" : "s"}`);
    for (const error of result.errors) {
      console.log(`  ${describe(error)}`);
      counts.set(error.kind, (counts.get(error.kind) ?? 0) + 1);
    }
    continue;
  }
  loaded++;
  const { config } = result;
  console.log(`${name}: ok`);
  console.log(`  name = ${config.name}`);
  console.log(`  port = ${config.port}`);
  console.log(`  debug = ${config.debug}`);
  console.log(`  timeout = ${config.timeout}`);
}
console.log(`files loaded: ${loaded} of ${files.length}`);
console.log(KINDS.map((kind) => `${kind}: ${counts.get(kind)}`).join(", "));
