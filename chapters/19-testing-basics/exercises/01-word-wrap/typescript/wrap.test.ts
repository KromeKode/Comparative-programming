// Write tests for wrap here, with test(). One is here to start with, and the
// README lists what wrap promises.
import assert from "node:assert/strict";
import { test } from "node:test";
import { wrap } from "./wrap.ts";

test("one line", () => {
  assert.deepEqual(wrap("one two", 10), ["one two"]);
});
