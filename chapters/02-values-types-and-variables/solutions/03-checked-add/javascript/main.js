import { readFileSync } from "node:fs";

// Number would silently round these values, so parse straight to BigInt.
const INT64_MIN = -(2n ** 63n);
const INT64_MAX = 2n ** 63n - 1n;

for (const line of readFileSync(0, "utf8").split("\n")) {
  const parts = line.trim().split(/\s+/);
  if (parts.length !== 2) {
    continue; // a blank line
  }
  const sum = BigInt(parts[0]) + BigInt(parts[1]);
  console.log(sum < INT64_MIN || sum > INT64_MAX ? "overflow" : String(sum));
}
