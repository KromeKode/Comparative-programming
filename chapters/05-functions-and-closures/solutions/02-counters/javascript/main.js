import { readFileSync } from "node:fs";

function makeCounter(start, step) {
  let next = start; // captured by the closure, and kept alive by it
  return () => {
    const value = next;
    next += step;
    return value;
  };
}

const counters = new Map();
for (const line of readFileSync(0, "utf8").split("\n")) {
  const [command, name, start, step] = line.trim().split(/\s+/);
  if (command === "new") {
    counters.set(name, makeCounter(Number(start), Number(step)));
  } else if (command === "next") {
    const counter = counters.get(name);
    console.log(counter ? `${name}: ${counter()}` : `${name}: no such counter`);
  }
}
