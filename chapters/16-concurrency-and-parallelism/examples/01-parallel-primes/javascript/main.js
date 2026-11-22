import {
  Worker,
  isMainThread,
  parentPort,
  workerData,
} from "node:worker_threads";

const LIMIT = 200_000;
const WORKERS = 4;

function isPrime(n) {
  if (n < 2) return false;
  for (let d = 2; d * d <= n; d++) {
    if (n % d === 0) return false;
  }
  return true;
}

function countPrimes({ start, end }) {
  let count = 0;
  for (let n = start; n < end; n++) {
    if (isPrime(n)) count++;
  }
  return count;
}

// A worker is a separate JavaScript thread with its own variables. It gets
// its input as a copy, workerData, and answers with a message.
function runWorker(job) {
  const worker = new Worker(new URL(import.meta.url), { workerData: job });
  return new Promise((resolve, reject) => {
    worker.once("message", resolve);
    worker.once("error", reject);
  });
}

// The same file runs on the main thread and in every worker.
if (isMainThread) {
  console.log(`counting primes below ${LIMIT} with ${WORKERS} workers`);
  const jobs = [];
  for (let i = 0; i < WORKERS; i++) {
    jobs.push({
      start: Math.floor((LIMIT * i) / WORKERS),
      end: Math.floor((LIMIT * (i + 1)) / WORKERS),
    });
  }
  // Promise.all waits for every worker, and keeps the results in order.
  const counts = await Promise.all(jobs.map(runWorker));
  for (const [i, { start, end }] of jobs.entries()) {
    console.log(`worker ${i}: [${start}, ${end}) has ${counts[i]} primes`);
  }
  console.log(`total: ${counts.reduce((a, b) => a + b, 0)} primes`);
} else {
  parentPort.postMessage(countPrimes(workerData));
}
