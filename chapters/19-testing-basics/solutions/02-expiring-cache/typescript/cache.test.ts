import assert from "node:assert/strict";
import { test } from "node:test";
import { Cache, type Clock } from "./cache.ts";

// A clock that says whatever time the test sets.
class FakeClock implements Clock {
  time = 0;
  now(): number {
    return this.time;
  }
}

test("put then get", () => {
  const cache = new Cache(10, new FakeClock());
  cache.put("a", "1");
  assert.equal(cache.get("a"), "1");
  assert.equal(cache.get("b"), undefined);
  assert.equal(cache.size, 1);
});

// An entry put at time t is there until t + ttl, and gone from then on.
test("expires after ttl", () => {
  const clock = new FakeClock();
  clock.time = 100;
  const cache = new Cache(10, clock);
  cache.put("a", "1");
  clock.time = 109;
  assert.equal(cache.get("a"), "1");
  clock.time = 110;
  assert.equal(cache.get("a"), undefined);
  assert.equal(cache.size, 0);
});

// Getting an entry doesn't make it last longer.
test("get does not refresh", () => {
  const clock = new FakeClock();
  const cache = new Cache(10, clock);
  cache.put("a", "1");
  clock.time = 5;
  assert.equal(cache.get("a"), "1");
  clock.time = 10;
  assert.equal(cache.get("a"), undefined);
});

// Putting a key again replaces its value, and its ttl starts again.
test("put replaces", () => {
  const clock = new FakeClock();
  const cache = new Cache(10, clock);
  cache.put("a", "1");
  clock.time = 5;
  cache.put("a", "2");
  assert.equal(cache.size, 1);
  clock.time = 14;
  assert.equal(cache.get("a"), "2");
  clock.time = 15;
  assert.equal(cache.get("a"), undefined);
});

// Each key has its own time: one expiring, or a put of another, leaves the rest.
test("keys are separate", () => {
  const clock = new FakeClock();
  const cache = new Cache(10, clock);
  cache.put("a", "1");
  clock.time = 5;
  cache.put("b", "2");
  assert.equal(cache.get("a"), "1");
  assert.equal(cache.get("b"), "2");
  assert.equal(cache.size, 2);
  clock.time = 10;
  assert.equal(cache.get("a"), undefined);
  assert.equal(cache.get("b"), "2");
  assert.equal(cache.size, 1);
});
