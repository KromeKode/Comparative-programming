import { createInterface } from "node:readline";

class LineError extends Error {}

class Version {
  constructor(major, minor, patch) {
    this.major = major;
    this.minor = minor;
    this.patch = patch;
  }

  // Part by part, as numbers: major first, then minor, then patch.
  static compare(a, b) {
    return a.major - b.major || a.minor - b.minor || a.patch - b.patch;
  }

  toString() {
    return `${this.major}.${this.minor}.${this.patch}`;
  }
}

// < on two Versions would compare their text, where "1.10.0" < "1.9.0". So
// merge takes a compare function, as sort does.
function merge(intervals, compare) {
  const merged = [];
  for (const interval of intervals.toSorted((a, b) => compare(a.low, b.low))) {
    const last = merged.at(-1);
    if (last !== undefined && compare(interval.low, last.high) <= 0) {
      if (compare(interval.high, last.high) > 0) last.high = interval.high;
    } else {
      merged.push({ ...interval });
    }
  }
  return merged;
}

const byNumber = (a, b) => a - b;
const byText = (a, b) => (a < b ? -1 : a > b ? 1 : 0);

function parseInt(text) {
  return /^-?[0-9]+$/.test(text) ? Number(text) : undefined;
}

function parseWord(text) {
  return /^[a-z]+$/.test(text) ? text : undefined;
}

function parseVersion(text) {
  const parts = text.split(".");
  if (parts.length !== 3 || !parts.every((part) => /^[0-9]+$/.test(part)))
    return undefined;
  const [major, minor, patch] = parts.map(Number);
  return new Version(major, minor, patch);
}

// Parse every interval on a line, merge them, and format the result.
function run(kind, parse, compare, words) {
  const intervals = words.map((word) => {
    const ends = word.split("..");
    if (ends.length !== 2) throw new LineError(`bad interval "${word}"`);
    const [low, high] = ends.map(parse);
    if (low === undefined) throw new LineError(`bad ${kind} "${ends[0]}"`);
    if (high === undefined) throw new LineError(`bad ${kind} "${ends[1]}"`);
    if (compare(high, low) < 0) throw new LineError(`${word} is backwards`);
    return { low, high };
  });
  const parts = merge(intervals, compare).map(
    ({ low, high }) => `${low}..${high}`,
  );
  return [kind, ...parts].join(" ");
}

const kinds = new Map([
  ["int", (words) => run("int", parseInt, byNumber, words)],
  ["word", (words) => run("word", parseWord, byText, words)],
  ["version", (words) => run("version", parseVersion, Version.compare, words)],
]);

for await (const line of createInterface({ input: process.stdin })) {
  const [kind, ...rest] = line.split(/\s+/).filter((word) => word !== "");
  if (kind === undefined) continue;
  try {
    const handle = kinds.get(kind);
    if (handle === undefined) throw new LineError(`unknown kind "${kind}"`);
    console.log(handle(rest));
  } catch (error) {
    if (!(error instanceof LineError)) throw error;
    console.log(`error: ${error.message}`);
  }
}
