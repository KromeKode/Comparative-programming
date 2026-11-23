// Counts the words in its input, and prints the ten most common. It is slow on a long
// text, and section 20.7 runs it under a profiler to find out why.
import { readFileSync } from "node:fs";

// Gives the words of the text: runs of the letters a to z, in lowercase.
function splitWords(text) {
  return text.toLowerCase().match(/[a-z]+/g) ?? [];
}

// Gives the index of the word's count, or -1 if it has none.
function find(counts, word) {
  for (let i = 0; i < counts.length; i++) {
    if (counts[i][0] === word) return i;
  }
  return -1;
}

function countWords(words) {
  const counts = [];
  for (const word of words) {
    const i = find(counts, word);
    if (i === -1) {
      counts.push([word, 1]);
    } else {
      counts[i][1]++;
    }
  }
  return counts;
}

const counts = countWords(splitWords(readFileSync(0, "utf8")));
counts.sort((a, b) => b[1] - a[1] || (a[0] < b[0] ? -1 : a[0] > b[0] ? 1 : 0));
for (const [word, n] of counts.slice(0, 10)) {
  console.log(`${n} ${word}`);
}
