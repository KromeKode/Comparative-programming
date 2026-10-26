import { readFileSync } from "node:fs";

// fib(79) is past 2^53, where numbers start losing whole values, so use bigint.
const memo = new Map<number, bigint>();

function fib(n: number): bigint {
  if (n < 2) {
    return BigInt(n);
  }
  const known = memo.get(n);
  if (known !== undefined) {
    return known;
  }
  const value = fib(n - 1) + fib(n - 2);
  memo.set(n, value);
  return value;
}

for (const line of readFileSync(0, "utf8").split("\n")) {
  const text = line.trim();
  if (text !== "") {
    const n = Number(text);
    console.log(`fib(${n}) = ${fib(n)}`);
  }
}
