import assert from "node:assert/strict";
import { mock, test } from "node:test";
import { type Heater, regulate, type Sensor } from "./thermostat.ts";

// A stub: a sensor that reads whatever temperature the test gives it.
function stubSensor(celsius: number): Sensor {
  return { readCelsius: () => celsius };
}

// A spy: a heater that records the calls made to it, for the test to check.
function spyHeater(): Heater & { calls: string[] } {
  const calls: string[] = [];
  return {
    calls,
    turnOn: () => {
      calls.push("on");
    },
    turnOff: () => {
      calls.push("off");
    },
  };
}

test("too cold", () => {
  const heater = spyHeater();
  regulate(stubSensor(17), heater, 20);
  assert.deepEqual(heater.calls, ["on"]);
});

test("too hot", () => {
  const heater = spyHeater();
  regulate(stubSensor(23), heater, 20);
  assert.deepEqual(heater.calls, ["off"]);
});

test("close enough", () => {
  for (const celsius of [19, 20, 21]) {
    const heater = spyHeater();
    regulate(stubSensor(celsius), heater, 20);
    assert.deepEqual(heater.calls, [], `at ${celsius}`);
  }
});

// The same test as "too hot", with node:test's mock functions instead, which
// record their calls.
test("too hot, with mocks", () => {
  const heater = { turnOn: mock.fn(), turnOff: mock.fn() };
  regulate({ readCelsius: () => 23 }, heater, 20);
  assert.equal(heater.turnOff.mock.callCount(), 1);
  assert.equal(heater.turnOn.mock.callCount(), 0);
});
