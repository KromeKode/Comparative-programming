// Write tests for parseDuration and formatDuration here, with test(). One is here to
// start with, and the README lists what they promise.
import assert from "node:assert/strict";
import { test } from "node:test";
import { parseDuration } from "./duration.ts";

test("parse", () => {
  assert.equal(parseDuration("1h30m"), 5400);
});
