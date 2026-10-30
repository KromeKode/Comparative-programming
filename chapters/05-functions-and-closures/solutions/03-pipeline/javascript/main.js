import { readFileSync } from "node:fs";

function makeStep(op, n) {
  switch (op) {
    case "add":
      return (x) => x + n;
    case "sub":
      return (x) => x - n;
    case "mul":
      return (x) => x * n;
    case "min":
      return (x) => Math.min(x, n);
    case "max":
      return (x) => Math.max(x, n);
    default:
      throw new Error(`unknown step: ${op}`);
  }
}

for (const line of readFileSync(0, "utf8").split("\n")) {
  const words = line.trim().split(/\s+/);
  if (words[0] === "") {
    continue; // a blank line
  }
  const start = Number(words[0]);
  const steps = [];
  for (let i = 1; i + 1 < words.length; i += 2) {
    steps.push(makeStep(words[i], Number(words[i + 1])));
  }
  let value = start;
  for (const step of steps) {
    value = step(value);
  }
  console.log(`${start} -> ${value}`);
}
