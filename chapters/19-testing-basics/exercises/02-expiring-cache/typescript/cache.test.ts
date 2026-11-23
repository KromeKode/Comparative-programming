// Write tests for the cache here, with test(). One is here to start with, on the
// computer's clock, which can't show what happens as time passes without the test
// waiting for it. The README lists what the cache promises.
import assert from "node:assert/strict";
import { test } from "node:test";
import { Cache, SystemClock } from "./cache.ts";

test("put then get", () => {
  const cache = new Cache(60, new SystemClock());
  cache.put("a", "1");
  assert.equal(cache.get("a"), "1");
});
