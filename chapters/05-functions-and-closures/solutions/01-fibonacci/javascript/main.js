import { readFileSync } from "node:fs";

// fib(79) is past 2^53, where numbers start losing whole values, so use BigInt.
const memo = new Map();

function fib(n) {
  if (n < 2) {
    return BigInt(n);
  }
  if (!memo.has(n)) {
    memo.set(n, fib(n - 1) + fib(n - 2));
  }
  return memo.get(n);
}

for (const line of readFileSync(0, "utf8").split("\n")) {
  const text = line.trim();
  if (text !== "") {
    const n = Number(text);
    console.log(`fib(${n}) = ${fib(n)}`);
  }
}
