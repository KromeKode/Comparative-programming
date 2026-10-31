import { readFileSync } from "node:fs";

// null means "not set", which is different from every real value.
type Config = {
  host: string | null;
  port: number | null;
  timeout: number | null;
};

const config: Config = { host: null, port: null, timeout: null };
const highest = { port: 65535, timeout: 86400 };

// The number if text is digits only and from 1 to high, or else null.
function parseNumber(text: string, high: number): number | null {
  if (!/^[0-9]+$/.test(text)) return null;
  const value = Number(text);
  return value >= 1 && value <= high ? value : null;
}

const lines = readFileSync(0, "utf8").split("\n");
if (lines.at(-1) === "") lines.pop(); // the newline at the end starts no new line

lines.forEach((text, index) => {
  const number = index + 1;
  const line = text.trim();
  if (line === "" || line.startsWith("#")) return;
  const equals = line.indexOf("=");
  if (equals === -1) {
    console.log(`line ${number}: expected key = value`);
    return;
  }
  const key = line.slice(0, equals).trim();
  const value = line.slice(equals + 1).trim();
  if (key === "host") {
    config.host = value === "" ? null : value;
  } else if (key === "port" || key === "timeout") {
    // Inside this branch, TypeScript knows key is "port" | "timeout".
    const parsed = parseNumber(value, highest[key]);
    if (value === "") {
      config[key] = null;
    } else if (parsed === null) {
      console.log(`line ${number}: bad ${key} "${value}"`);
    } else {
      config[key] = parsed;
    }
  } else {
    console.log(`line ${number}: unknown key "${key}"`);
  }
});

console.log(`host: ${config.host ?? "localhost (default)"}`);
console.log(`port: ${config.port ?? "8080 (default)"}`);
const timeout = config.timeout === null ? "none" : `${config.timeout}s`;
console.log(`timeout: ${timeout}`);
