import { Worker, isMainThread, workerData } from "node:worker_threads";

const THREADS = 4;
const TIMES = 100_000;

// Every value sent to a worker is copied, except a SharedArrayBuffer, which
// is memory that every thread it is sent to can see. An Int32Array reads it
// as 32-bit integers.
function runWorker(shared) {
  const worker = new Worker(new URL(import.meta.url), { workerData: shared });
  return new Promise((resolve, reject) => {
    worker.once("exit", () => resolve());
    worker.once("error", reject);
  });
}

if (isMainThread) {
  console.log(`${THREADS} threads add 1, ${TIMES} times each`);
  const shared = new SharedArrayBuffer(4);
  await Promise.all(Array.from({ length: THREADS }, () => runWorker(shared)));
  console.log(`with an atomic: ${Atomics.load(new Int32Array(shared), 0)}`);
} else {
  // Atomics.add reads, adds, and writes as one step. JavaScript has no
  // mutex, but Atomics.wait and Atomics.notify are enough to build one.
  const count = new Int32Array(workerData);
  for (let i = 0; i < TIMES; i++) Atomics.add(count, 0, 1);
}
