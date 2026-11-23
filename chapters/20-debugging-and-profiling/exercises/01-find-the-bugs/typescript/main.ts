// Prints a report of each student's scores. ../README.md says what it should print.
import { readFileSync } from "node:fs";

// Gives total / count, rounded to the nearest whole number, with halves up.
function roundedAverage(total: number, count: number): number {
  return Math.floor((2 * total + count) / (2 * count));
}

// Each student's scores, in the order the students first appear.
const scores = new Map<string, number[]>();
// The input is the file named on the command line, if there is one, so that a
// debugger can keep the terminal to itself.
const input = readFileSync(process.argv[2] ?? 0, "utf8");
for (const line of input.split("\n")) {
  const [name, ...fields] = line.split(/\s+/).filter((field) => field !== "");
  if (name === undefined) continue;
  const marks = fields.map(Number);
  scores.set(name, marks);
}

let bestName = "-";
let bestAverage = -1;
let classTotal = 0;
let classCount = 0;
let total = 0;
for (const [name, marks] of scores) {
  for (const mark of marks) {
    total += mark;
  }
  classTotal += total;
  classCount += marks.length;
  const average = roundedAverage(total, marks.length);
  console.log(`${name} ${average}`);
  if (average >= bestAverage) {
    bestName = name;
    bestAverage = average;
  }
}

if (classCount === 0) {
  console.log("class average: -");
} else {
  console.log(`class average: ${roundedAverage(classTotal, classCount)}`);
}
console.log(`top: ${bestName}`);
