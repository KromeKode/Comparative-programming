import { readFileSync } from "node:fs";

// Returns the pair with the smallest j, and for that j the smallest i.
function twoSum(target: number, numbers: number[]): [number, number] | null {
  const firstIndex = new Map<number, number>();
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
  const rest = line.slice(colon + 1);
  const numbers = rest.split(/\s+/).filter(Boolean).map(Number); // Boolean drops ""
  const pair = twoSum(target, numbers);
  console.log(pair ? pair.join(" ") : "none");
}
