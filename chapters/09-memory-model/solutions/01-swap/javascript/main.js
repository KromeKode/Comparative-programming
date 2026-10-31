import { readFileSync } from "node:fs";

// A function cannot reach the caller's variables, only the objects they refer
// to, so swap takes the array and two indexes.
function swap(words, i, j) {
  [words[i], words[j]] = [words[j], words[i]];
}

// Reverses words[start..end] and returns how many swaps that took.
function reverse(words, start, end) {
  let swaps = 0;
  for (; start < end; start++, end--) {
    swap(words, start, end);
    swaps++;
  }
  return swaps;
}

function parsePosition(text, count) {
  if (!/^[0-9]+$/.test(text)) {
    console.log(`error: "${text}" is not a position`);
    return null;
  }
  const value = Number(text);
  if (value < 1 || value > count) {
    console.log(`error: position ${text} is out of range (1 to ${count})`);
    return null;
  }
  return value;
}

const lines = readFileSync(0, "utf8").split("\n");
const words = lines[0].split(/\s+/).filter((w) => w !== "");
let swaps = 0;
for (const line of lines.slice(1)) {
  const parts = line.split(/\s+/).filter((p) => p !== "");
  if (parts.length === 0) continue;
  const command = parts[0];
  if (command !== "swap" && command !== "reverse") {
    console.log(`error: unknown command "${command}"`);
    continue;
  }
  if (parts.length !== 3) {
    console.log(`error: ${command} needs 2 positions, got ${parts.length - 1}`);
    continue;
  }
  const i = parsePosition(parts[1], words.length);
  if (i === null) continue;
  const j = parsePosition(parts[2], words.length);
  if (j === null) continue;
  if (command === "swap") {
    swap(words, i - 1, j - 1);
    swaps++;
  } else {
    swaps += reverse(words, Math.min(i, j) - 1, Math.max(i, j) - 1);
  }
  console.log(words.join(" "));
}
console.log(`swaps: ${swaps}`);
