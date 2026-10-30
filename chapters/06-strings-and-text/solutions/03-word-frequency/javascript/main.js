import { readFileSync } from "node:fs";

const text = readFileSync(0, "utf8");

// A word is a run of ASCII letters and non-ASCII characters. The u flag makes the
// regex match whole code points, so an emoji is one character, not two halves.
const words = text.match(/[A-Za-z\u0080-\u{10FFFF}]+/gu) ?? [];

const counts = new Map();
for (const word of words) {
  const lower = word.replace(/[A-Z]/g, (c) => c.toLowerCase()); // ASCII letters only
  counts.set(lower, (counts.get(lower) ?? 0) + 1);
}

// Most frequent first; ties in UTF-16 code unit order, which is how < compares strings.
const sorted = [...counts].sort(
  ([a, m], [b, n]) => n - m || (a < b ? -1 : a > b ? 1 : 0),
);
for (const [word, count] of sorted) {
  console.log(`${count} ${word}`);
}
