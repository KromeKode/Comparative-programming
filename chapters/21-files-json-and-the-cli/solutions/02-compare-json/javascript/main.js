// Prints how one JSON file differs from another: what was removed, what was added, and
// what changed. ../README.md says what it should print.
import { readFileSync } from "node:fs";

const USAGE = "usage: jsondiff OLD NEW";
const IDENTIFIER = /^[A-Za-z_][A-Za-z0-9_]*$/;
// Node's errors have the C names of the system's error codes, such as ENOENT, and
// messages of their own; these are the messages strerror gives for them.
const MESSAGES = { ENOENT: "No such file or directory", EISDIR: "Is a directory" };

// typeof says "object" for null and for arrays too.
function isObject(value) {
  return value !== null && typeof value === "object" && !Array.isArray(value);
}

// An object's keys in order of their characters' codes. Object.keys puts keys that look
// like array indexes, such as "9" and "10", first, in the order of their numbers.
function sortedKeys(...objects) {
  return [...new Set(objects.flatMap((object) => Object.keys(object)))].sort();
}

function compact(value) {
  if (Array.isArray(value)) {
    return `[${value.map(compact).join(",")}]`;
  }
  if (isObject(value)) {
    const members = sortedKeys(value).map((key) => `${JSON.stringify(key)}:${compact(value[key])}`);
    return `{${members.join(",")}}`;
  }
  return JSON.stringify(value);
}

// The part of a path that leads to a key: .name, or ["first name"].
function step(key) {
  return IDENTIFIER.test(key) ? `.${key}` : `[${JSON.stringify(key)}]`;
}

// Adds a line to out for each difference between before and after.
function diff(path, before, after, out) {
  if (isObject(before) && isObject(after)) {
    for (const key of sortedKeys(before, after)) {
      const here = path + step(key);
      // Object.hasOwn, not "key in after", which is true for "constructor" in any object.
      if (!Object.hasOwn(after, key)) {
        out.push(`- ${here}: ${compact(before[key])}`);
      } else if (!Object.hasOwn(before, key)) {
        out.push(`+ ${here}: ${compact(after[key])}`);
      } else {
        diff(here, before[key], after[key], out);
      }
    }
  } else if (Array.isArray(before) && Array.isArray(after)) {
    for (let i = 0; i < Math.max(before.length, after.length); i++) {
      const here = `${path}[${i}]`;
      if (i >= after.length) {
        out.push(`- ${here}: ${compact(before[i])}`);
      } else if (i >= before.length) {
        out.push(`+ ${here}: ${compact(after[i])}`);
      } else {
        diff(here, before[i], after[i], out);
      }
    }
  } else if (before !== after) {
    // Two objects or two arrays went the ways above, so at least one of before and after
    // is a string, number, boolean, or null, which === compares by value.
    out.push(`~ ${path}: ${compact(before)} -> ${compact(after)}`);
  }
}

function main() {
  const paths = process.argv.slice(2);
  if (paths.length !== 2) {
    console.error(USAGE);
    return 2;
  }
  const values = [];
  for (const path of paths) {
    let text;
    try {
      text = readFileSync(path, "utf8");
    } catch (error) {
      console.error(`jsondiff: ${path}: ${MESSAGES[error.code] ?? error.message}`);
      return 2;
    }
    try {
      values.push(JSON.parse(text));
    } catch {
      console.error(`jsondiff: ${path}: not valid JSON`);
      return 2;
    }
  }
  const lines = [];
  diff("$", values[0], values[1], lines);
  process.stdout.write(lines.map((line) => line + "\n").join(""));
  return lines.length > 0 ? 1 : 0;
}

process.exitCode = main();
