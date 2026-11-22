import { Worker, isMainThread, parentPort } from "node:worker_threads";

const COUNT = 1000;
const WORKERS = 3;

if (isMainThread) {
  // Workers share no memory with the main thread. postMessage sends a copy
  // of a value, and the other side gets it as a "message" event. Only the
  // main thread hands out jobs, so no two workers can get the same one.
  let next = 1;
  let received = 0;
  let sum = 0;

  function startWorker(): Promise<void> {
    const worker = new Worker(new URL(import.meta.url));
    worker.on("message", (square: number | null) => {
      if (square !== null) {
        received++;
        sum += square;
      }
      // Each answer asks for the next job, so a faster worker gets more.
      worker.postMessage(next <= COUNT ? next++ : null);
    });
    return new Promise((resolve, reject) => {
      worker.once("exit", () => resolve());
      worker.once("error", reject);
    });
  }

  await Promise.all(Array.from({ length: WORKERS }, startWorker));
  console.log(`${WORKERS} workers squared 1 to ${COUNT}`);
  console.log(`results: ${received}`);
  console.log(`sum: ${sum}`);
} else if (parentPort) {
  // A worker asks for its first job with null, then answers each job with
  // its result. null means stop, and closing the port lets the worker end.
  const port = parentPort;
  port.on("message", (n: number | null) => {
    if (n === null) port.close();
    else port.postMessage(n * n);
  });
  port.postMessage(null);
}
