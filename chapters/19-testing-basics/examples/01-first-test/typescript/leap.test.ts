// Node strips the types and runs this file as it is; tsc checks them first.
import assert from "node:assert/strict";
import { test } from "node:test";
import { isLeapYear } from "./leap.ts";

test("divisible by 4", () => {
  assert.equal(isLeapYear(2024), true);
  assert.equal(isLeapYear(2023), false);
});

test("centuries", () => {
  assert.equal(isLeapYear(1900), false);
  assert.equal(isLeapYear(2000), true);
});
