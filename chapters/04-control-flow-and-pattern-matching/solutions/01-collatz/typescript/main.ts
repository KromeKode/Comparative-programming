import { readFileSync } from "node:fs";

for (const line of readFileSync(0, "utf8").split("\n")) {
  const text = line.trim();
  if (text === "") {
    continue;
  }
  const start = Number(text);
  // The peaks stay far below 2^53, so a plain number is exact here.
  let n: number = start;
  let peak: number = n;
  let steps = 0;
  while (n !== 1) {
    n = n % 2 === 0 ? n / 2 : 3 * n + 1;
    peak = Math.max(peak, n);
    steps++;
  }
  console.log(`${start}: steps ${steps}, peak ${peak}`);
}
