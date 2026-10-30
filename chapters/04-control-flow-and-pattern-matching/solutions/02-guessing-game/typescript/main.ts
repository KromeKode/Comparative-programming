import { readFileSync } from "node:fs";

const guessesText = (count: number): string => (count === 1 ? "1 guess" : `${count} guesses`);

const [first, ...rest] = readFileSync(0, "utf8").split("\n");
const secret = Number(first);

let guesses = 0;
let solved = false;
for (const line of rest) {
  const text = line.trim();
  if (text === "") {
    continue; // a blank line
  }
  const guess = Number(text);
  if (!Number.isInteger(guess)) {
    // Number gives NaN for "fifty" and 3.5 for "3.5"; neither is a whole number.
    console.log(`${text}: not a number`);
    continue;
  }
  guesses++;
  if (guess < secret) {
    console.log(`${guess}: too low`);
  } else if (guess > secret) {
    console.log(`${guess}: too high`);
  } else {
    console.log(`${guess}: correct in ${guessesText(guesses)}`);
    solved = true;
    break;
  }
}
if (!solved) {
  console.log(`gave up after ${guessesText(guesses)}; the number was ${secret}`);
}
