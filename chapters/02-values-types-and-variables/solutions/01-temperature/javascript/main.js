import { readFileSync } from "node:fs";

for (const line of readFileSync(0, "utf8").split("\n")) {
  const parts = line.trim().split(/\s+/);
  if (parts.length !== 2) {
    continue; // a blank line
  }
  const value = Number(parts[0]);
  if (parts[1] === "C") {
    console.log(`${value.toFixed(1)} C = ${((value * 9) / 5 + 32).toFixed(1)} F`);
  } else {
    console.log(`${value.toFixed(1)} F = ${(((value - 32) * 5) / 9).toFixed(1)} C`);
  }
}
