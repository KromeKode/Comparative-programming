import { readFileSync } from "node:fs";

const isLeap = (year) => year % 4 === 0 && (year % 100 !== 0 || year % 400 === 0);

for (const line of readFileSync(0, "utf8").split("\n")) {
  const text = line.trim();
  if (text === "") {
    continue;
  }
  const year = Number(text);
  console.log(`${year}: ${isLeap(year) ? "leap year" : "common year"}`);
}
