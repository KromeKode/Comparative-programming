import assert from "node:assert/strict";
import { test } from "node:test";
import { toRoman } from "./roman.js";

const cases = [
  [1, "I"],
  [4, "IV"],
  [9, "IX"],
  [14, "XIV"],
  [40, "XL"],
  [90, "XC"],
  [400, "CD"],
  [1994, "MCMXCIV"],
  [2024, "MMXXIV"],
  [3999, "MMMCMXCIX"],
];

// A test for each case, named after it, so a failure says which case it was.
for (const [n, want] of cases) {
  test(`toRoman(${n})`, () => {
    assert.equal(toRoman(n), want);
  });
}

// The errors are part of what toRoman promises, so they get tests too.
test("out of range", () => {
  for (const n of [0, 4000, -1]) {
    assert.throws(() => toRoman(n), RangeError);
  }
});
