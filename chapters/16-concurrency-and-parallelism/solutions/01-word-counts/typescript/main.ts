import { readFileSync } from "node:fs";
import {
  Worker,
  isMainThread,
  parentPort,
  workerData,
} from "node:worker_threads";

type Counts = Map<string, number>;

function plural(count: number, noun: string): string {
  return `${count} ${noun}${count === 1 ? "" : "s"}`;
}

// Runs in a worker, which has its own copy of the lines, and sends its Map
// back as a message, which copies it again.
function countWords(lines: string[]): Counts {
  const counts: Counts = new Map();
  for (const line of lines) {
    for (const match of line.match(/[A-Za-z]+/g) ?? []) {
      const word = match.toLowerCase();
      counts.set(word, (counts.get(word) ?? 0) + 1);
    }
  }
  return counts;
}

// A message arrives as any, so the Promise says what it holds.
function runWorker(lines: string[]): Promise<Counts> {
  const worker = new Worker(new URL(import.meta.url), { workerData: lines });
  return new Promise((resolve, reject) => {
    worker.once("message", resolve);
    worker.once("error", reject);
  });
}

const sum = (values: number[]): number => values.reduce((a, b) => a + b, 0);

async function main(): Promise<void> {
  const lines = readFileSync(0, "utf8").split("\n");
  if (lines.at(-1) === "") lines.pop(); // after the newline that ends the input
  const first = lines.length > 0 ? lines[0].trim().split(/\s+/) : [];
  if (first.length !== 2 || !first.every((word) => /^[0-9]+$/.test(word))) {
    console.log("error: the first line must be two numbers, workers and top");
    return;
  }
  const [workers, top] = first.map(Number);
  if (workers < 1 || workers > 16) {
    console.log("error: workers must be from 1 to 16");
    return;
  }

  // Each worker gets text.length / workers lines, and the first few get one
  // more, so that every line is counted once.
  const text = lines.slice(1);
  const chunks: string[][] = [];
  let start = 0;
  for (let i = 0; i < workers; i++) {
    const size =
      Math.floor(text.length / workers) + (i < text.length % workers ? 1 : 0);
    chunks.push(text.slice(start, start + size));
    start += size;
  }

  // Promise.all waits for every worker, and keeps the results in order.
  const counts = await Promise.all(chunks.map(runWorker));

  console.log(`${plural(workers, "worker")}, ${plural(text.length, "line")}`);
  const total: Counts = new Map();
  for (const [i, chunk] of chunks.entries()) {
    const words = sum([...counts[i].values()]);
    console.log(
      `worker ${i}: ${plural(chunk.length, "line")}, ${plural(words, "word")}`,
    );
    for (const [word, count] of counts[i])
      total.set(word, (total.get(word) ?? 0) + count);
  }
  console.log(
    `${plural(sum([...total.values()]), "word")}, ${total.size} different`,
  );

  const sorted = [...total].sort(
    ([a, x], [b, y]) => y - x || (a < b ? -1 : a > b ? 1 : 0),
  );
  for (const [word, count] of sorted.slice(0, top))
    console.log(`${word}: ${count}`);
}

// The same file runs on the main thread and in every worker.
if (isMainThread) {
  await main();
} else {
  parentPort?.postMessage(countWords(workerData as string[]));
}
