import { readFileSync } from "node:fs";

// Number would silently round these values, so parse straight to BigInt.
const INT64_MIN: bigint = -(2n ** 63n);
const INT64_MAX: bigint = 2n ** 63n - 1n;

for (const line of readFileSync(0, "utf8").split("\n")) {
  const parts: string[] = line.trim().split(/\s+/);
  if (parts.length !== 2) {
    continue; // a blank line
  }
  const sum: bigint = BigInt(parts[0]) + BigInt(parts[1]);
  console.log(sum < INT64_MIN || sum > INT64_MAX ? "overflow" : String(sum));
}
