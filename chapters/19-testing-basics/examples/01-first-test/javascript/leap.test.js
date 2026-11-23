// node --test runs files whose names end in .test.js, among others. Each call to
// test() registers a test, which the runner runs after the file has loaded.
import assert from "node:assert/strict";
import { test } from "node:test";
import { isLeapYear } from "./leap.js";

test("divisible by 4", () => {
  assert.equal(isLeapYear(2024), true);
  assert.equal(isLeapYear(2023), false);
});

test("centuries", () => {
  assert.equal(isLeapYear(1900), false);
  assert.equal(isLeapYear(2000), true);
});
