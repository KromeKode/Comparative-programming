import { readFileSync } from "node:fs";

// Each adapter is a generator: it takes any iterable of numbers and pulls one
// item at a time, keeping only what it needs.
function* windows(items, k) {
  const window = [];
  for (const item of items) {
    window.push(item);
    if (window.length > k) window.shift(); // the oldest item leaves
    if (window.length === k) yield [...window]; // a copy, since window keeps changing
  }
}

function* chunks(items, k) {
  let chunk = [];
  for (const item of items) {
    chunk.push(item);
    if (chunk.length === k) {
      yield chunk;
      chunk = [];
    }
  }
  if (chunk.length > 0) yield chunk;
}

function* runs(items) {
  let run = [];
  for (const item of items) {
    if (run.length > 0 && item !== run[0]) {
      yield run;
      run = [];
    }
    run.push(item);
  }
  if (run.length > 0) yield run;
}

const show = (groups) =>
  groups
    .map((group) => `[${group.join(" ")}]`)
    .toArray()
    .join(" ");

function answer(words) {
  let text;
  if (words[0] === "runs") {
    text = show(runs(words.slice(1).values().map(Number)));
  } else {
    const k = Number(words[1]);
    const numbers = words.slice(2).values().map(Number);
    if (words[0] === "windows") text = show(windows(numbers, k));
    else if (words[0] === "chunks") text = show(chunks(numbers, k));
    else {
      const sums = windows(numbers, k).map((window) =>
        window.reduce((a, b) => a + b, 0),
      );
      text = sums.toArray().join(" ");
    }
  }
  return text || "(none)";
}

for (const line of readFileSync(0, "utf8").split("\n")) {
  const words = line.split(/\s+/).filter(Boolean);
  if (words.length > 0) console.log(answer(words));
}
