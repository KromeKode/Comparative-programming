import { readFileSync } from "node:fs";

// A JS number holds whole numbers exactly up to 2^53. Beyond that, use BigInt (chapter 2).
let total = 0;
for (const line of readFileSync(0, "utf8").split("\n")) {
  const trimmed = line.trim();
  if (trimmed === "") {
    continue;
  }
  total += Number(trimmed);
}
console.log(total);
