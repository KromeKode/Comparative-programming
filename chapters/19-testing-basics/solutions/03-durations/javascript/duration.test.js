import assert from "node:assert/strict";
import { test } from "node:test";
import { formatDuration, parseDuration } from "./duration.js";

const parses = [
  ["45s", 45],
  ["1h30m", 5400],
  ["2d", 172800],
  ["1d2h3m4s", 93784],
  // A number can be more than the next unit up holds, and it can be 0.
  ["90m", 5400],
  ["0s", 0],
  ["1h0m", 3600],
];
for (const [text, seconds] of parses) {
  test(`parseDuration("${text}")`, () => {
    assert.equal(parseDuration(text), seconds);
  });
}

const rejects = [
  "", // no parts
  "90", // a number with no unit
  "1h30", // the same, after a part
  "h", // a unit with no number
  "1x", // not a unit
  "1h1h", // a unit twice
  "30m1h", // units out of order
  "1h 30m", // anything else
  "-1h",
];
for (const text of rejects) {
  test(`parseDuration("${text}") throws`, () => {
    assert.throws(() => parseDuration(text), SyntaxError);
  });
}

const formats = [
  [5400, "1h30m"],
  [93784, "1d2h3m4s"],
  [59, "59s"],
  // Units that are 0 are left out, wherever they are, unless everything is.
  [3600, "1h"],
  [86401, "1d1s"],
  [0, "0s"],
];
for (const [seconds, text] of formats) {
  test(`formatDuration(${seconds})`, () => {
    assert.equal(formatDuration(seconds), text);
  });
}

test("round trip", () => {
  for (let seconds = 0; seconds < 100_000; seconds++) {
    assert.equal(parseDuration(formatDuration(seconds)), seconds);
  }
});
