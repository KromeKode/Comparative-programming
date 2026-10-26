import { readFileSync } from "node:fs";

for (const line of readFileSync(0, "utf8").split("\n")) {
  const parts = line.trim().split(/\s+/);
  if (parts.length !== 2) {
    continue; // a blank line
  }
  const a = Number(parts[0]);
  const b = Number(parts[1]);
  // Below 2^31 the float quotient is precise enough to round correctly.
  const q = Math.trunc(a / b);
  const r = a % b; // % truncates, like C
  const fq = Math.floor(a / b);
  const fm = a - b * fq;
  // A template string prints -0 as "0"; console.log(-0) would print "-0".
  console.log(`${a} ${b} -> truncated ${q} ${r}, floored ${fq} ${fm}`);
}
