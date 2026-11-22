// Versions and ranges as npm writes them: 1.2.3-beta.1, ^1.2, ~1.2.3, >=1 <2, 1.x || 3.
//
// Only what this file exports can be imported: Version, parseVersion, compare,
// Range, and parseRange. Everything else is private to the module.

const NUMBER = "0|[1-9][0-9]{0,14}";
const IDENTIFIER = "(?:0|[1-9][0-9]*|[0-9]*[A-Za-z-][0-9A-Za-z-]*)";
const PRE = `${IDENTIFIER}(?:\\.${IDENTIFIER})*`;
const VERSION = new RegExp(`^(${NUMBER})\\.(${NUMBER})\\.(${NUMBER})(?:-(${PRE}))?$`);
const PART = `(?:${NUMBER}|[xX*])`;
// A partial version, 1 or 1.2 or 1.2.3, with x, X, or * for any part. A pre-release
// can follow only three numbers, which expand checks.
const PARTIAL = new RegExp(`^(${PART})(?:\\.(${PART})(?:\\.(${PART})(?:-(${PRE}))?)?)?$`);
const OPERATOR = /^(<=|>=|<|>|=|\^|~)?/;
const isWild = (part: string): boolean => part === "x" || part === "X" || part === "*";

export class Version {
  major: number;
  minor: number;
  patch: number;
  pre: string[]; // the pre-release's identifiers, if it has one

  constructor(major: number, minor: number, patch: number, pre: string[] = []) {
    this.major = major;
    this.minor = minor;
    this.patch = patch;
    this.pre = pre;
  }

  toString(): string {
    const text = `${this.major}.${this.minor}.${this.patch}`;
    return this.pre.length > 0 ? `${text}-${this.pre.join(".")}` : text;
  }
}

// The version that text is, or null if it isn't one.
export function parseVersion(text: string): Version | null {
  const match = VERSION.exec(text);
  if (!match) return null;
  const [, major, minor, patch, pre] = match;
  return new Version(Number(major), Number(minor), Number(patch), pre ? pre.split(".") : []);
}

const order = <T>(a: T, b: T): number => (a < b ? -1 : a > b ? 1 : 0);
const isNumber = (text: string): boolean => /^[0-9]+$/.test(text);

// Numbers compare as numbers, and come before other identifiers, which compare
// character by character. Numbers have no 0 in front, so a longer one is larger.
function compareIdentifiers(a: string, b: string): number {
  if (isNumber(a) && isNumber(b)) return order(a.length, b.length) || order(a, b);
  if (isNumber(a) !== isNumber(b)) return isNumber(a) ? -1 : 1;
  return order(a, b);
}

const sameCore = (a: Version, b: Version): boolean =>
  a.major === b.major && a.minor === b.minor && a.patch === b.patch;

// Negative if a comes before b, 0 if they are equal, and positive if after.
export function compare(a: Version, b: Version): number {
  const core = order(a.major, b.major) || order(a.minor, b.minor) || order(a.patch, b.patch);
  if (core !== 0) return core;
  // A version without a pre-release comes after the same version with one.
  if (a.pre.length === 0 || b.pre.length === 0) return order(b.pre.length, a.pre.length);
  for (let i = 0; i < a.pre.length && i < b.pre.length; i++) {
    const result = compareIdentifiers(a.pre[i], b.pre[i]);
    if (result !== 0) return result;
  }
  return order(a.pre.length, b.pre.length);
}

type Op = "=" | "<" | "<=" | ">" | ">=";
// A simple comparator: an operator and a full version.
type Simple = [Op, Version];

// The simple comparators that a comparator stands for, or null if it isn't one.
function expand(token: string): Simple[] | null {
  const op = OPERATOR.exec(token)![0];
  const match = PARTIAL.exec(token.slice(op.length));
  if (!match) return null;
  const parts = match.slice(1, 4);
  // known counts the numbers, which must come before any wildcards.
  let known = 0;
  while (known < 3 && parts[known] !== undefined && !isWild(parts[known])) known++;
  if (parts.slice(known).some((part) => part !== undefined && !isWild(part))) return null;
  // A pre-release can follow only three numbers.
  if (match[4] !== undefined && known < 3) return null;
  const nums = [0, 0, 0].map((zero, i) => (i < known ? Number(parts[i]) : zero));
  const low = new Version(nums[0], nums[1], nums[2], match[4] ? match[4].split(".") : []);
  // The first version after all those whose parts up to level match: the -0
  // pre-release of the next major, minor, or patch.
  const after = (level: number): Version => {
    if (level === 0) return new Version(nums[0] + 1, 0, 0, ["0"]);
    if (level === 1) return new Version(nums[0], nums[1] + 1, 0, ["0"]);
    return new Version(nums[0], nums[1], nums[2] + 1, ["0"]);
  };
  if (known === 0) {
    // Any version: nothing is outside it, and nothing is above or below it.
    return op === "<" || op === ">" ? [["<", new Version(0, 0, 0, ["0"])]] : [];
  }
  const full = known === 3;
  const next = after(known - 1);
  switch (op) {
    case "":
    case "=":
      return full ? [["=", low]] : [[">=", low], ["<", next]];
    case ">":
      return full ? [[">", low]] : [[">=", new Version(next.major, next.minor, next.patch)]];
    case ">=":
      return [[">=", low]];
    case "<":
      return [["<", full ? low : new Version(nums[0], nums[1], nums[2], ["0"])]];
    case "<=":
      return full ? [["<=", low]] : [["<", next]];
    case "~":
      return [[">=", low], ["<", after(known === 1 ? 0 : 1)]];
    default: {
      // ^ allows changes that don't change the first part that isn't 0.
      const first = nums.slice(0, known).findIndex((num) => num !== 0);
      return [[">=", low], ["<", after(first >= 0 ? first : known - 1)]];
    }
  }
}

function holds(version: Version, [op, bound]: Simple): boolean {
  const result = compare(version, bound);
  switch (op) {
    case "=":
      return result === 0;
    case "<":
      return result < 0;
    case "<=":
      return result <= 0;
    case ">":
      return result > 0;
    default:
      return result >= 0;
  }
}

export class Range {
  // Sets of simple comparators joined by ||. A version is in the range if it passes
  // every comparator of some set. # makes the field private to the class.
  #sets: Simple[][];

  constructor(sets: Simple[][]) {
    this.#sets = sets;
  }

  allows(version: Version): boolean {
    return this.#sets.some(
      (simples) =>
        simples.every((simple) => holds(version, simple)) &&
        // A pre-release is in only if a comparator of the set names a pre-release
        // of the same major, minor, and patch.
        (version.pre.length === 0 ||
          simples.some(([, bound]) => bound.pre.length > 0 && sameCore(bound, version))),
    );
  }

  // The highest of versions that the range allows, or null if it allows none.
  highest(versions: Version[]): Version | null {
    let best: Version | null = null;
    for (const version of versions) {
      if (this.allows(version) && (best === null || compare(version, best) > 0)) best = version;
    }
    return best;
  }
}

// The range that text is. Throws a SyntaxError, saying why, if it isn't one.
export function parseRange(text: string): Range {
  const sets = text.split("||").map((side) => {
    const tokens = side.split(/\s+/).filter((token) => token !== "");
    if (tokens.length === 0) throw new SyntaxError("each side of || needs a comparator");
    return tokens.flatMap((token) => {
      const simples = expand(token);
      if (simples === null) throw new SyntaxError(`${token} isn't a comparator`);
      return simples;
    });
  });
  return new Range(sets);
}
