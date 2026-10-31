import { readFileSync } from "node:fs";

for (const line of readFileSync(0, "utf8").split("\n")) {
  const words = line.split(/\s+/).filter((word) => word !== "");
  if (words.length === 0) continue;
  // A Set keeps the first occurrence of each value, in insertion order.
  console.log([...new Set(words)].join(" "));
}
