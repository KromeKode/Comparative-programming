// Counts the letters in its input, and draws a bar for each one. It expects lowercase
// text, and doesn't check: section 20.3 gives it capitals, to see what happens.
import { readFileSync } from "node:fs";

function countLetters(text: string): number[] {
  const counts: number[] = new Array<number>(26).fill(0);
  for (const c of text) {
    if (/\p{L}/u.test(c)) {
      counts[c.codePointAt(0)! - 97]++;
    }
  }
  return counts;
}

const counts = countLetters(readFileSync(0, "utf8"));
counts.forEach((n, i) => {
  if (n > 0) {
    console.log(`${String.fromCharCode(97 + i)} ${n} ${"#".repeat(n)}`);
  }
});
