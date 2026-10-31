import { readFileSync } from "node:fs";

// Returns the pair with the smallest j, and for that j the smallest i.
function twoSum(target, numbers) {
  // Each number seen so far, mapped to the first index where it appeared.
  const firstIndex = new Map();
  for (const [j, n] of numbers.entries()) {
    const i = firstIndex.get(target - n);
    if (i !== undefined) return [i, j];
    if (!firstIndex.has(n)) firstIndex.set(n, j);
  }
  return null;
}

for (const line of readFileSync(0, "utf8").split("\n")) {
  const colon = line.indexOf(":");
  if (colon === -1) continue; // a blank line
  const target = Number(line.slice(0, colon));
  // A JavaScript number holds every integer up to 2^53 exactly, so these sums are safe.
  const rest = line.slice(colon + 1);
  const numbers = rest.split(/\s+/).filter(Boolean).map(Number); // Boolean drops ""
  const pair = twoSum(target, numbers);
  console.log(pair ? pair.join(" ") : "none");
}
