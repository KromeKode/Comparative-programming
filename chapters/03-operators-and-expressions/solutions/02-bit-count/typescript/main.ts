import { readFileSync } from "node:fs";

function countOnes(n: number): number {
  let count = 0;
  // >>> shifts in zeros. With >>, any n of 2^31 or more would look negative
  // and the loop would never end.
  for (let rest = n; rest !== 0; rest >>>= 1) {
    count += rest & 1;
  }
  return count;
}

for (const line of readFileSync(0, "utf8").split("\n")) {
  const text = line.trim();
  if (text === "") {
    continue;
  }
  const n = Number(text);
  const width = 32 - Math.clz32(n); // clz32 counts leading zero bits
  console.log(`${n}: ${n.toString(2)} (ones ${countOnes(n)}, width ${width})`);
}
