import { readFileSync } from "node:fs";

const n = Number.parseInt(readFileSync(0, "utf8"), 10);
if (Number.isNaN(n)) {
  console.error("expected a whole number");
  process.exit(1);
}

for (let i = 1; i <= n; i++) {
  if (i % 15 === 0) {
    console.log("FizzBuzz");
  } else if (i % 3 === 0) {
    console.log("Fizz");
  } else if (i % 5 === 0) {
    console.log("Buzz");
  } else {
    console.log(i);
  }
}
