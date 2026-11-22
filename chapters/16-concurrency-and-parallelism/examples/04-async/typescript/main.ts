// Resolves on a later turn of the event loop. A task that awaits it goes to
// the back of the line, behind everything that is ready to run.
const pause = (): Promise<void> =>
  new Promise((resolve) => setTimeout(resolve, 0));

async function task(name: string, steps: number): Promise<number> {
  for (let step = 1; step <= steps; step++) {
    console.log(`${name}: step ${step}`);
    // await is the only place a task can stop and let another one run.
    await pause();
  }
  console.log(`${name}: done`);
  return steps;
}

interface Counter {
  value: number;
}

async function addAtOnce(count: Counter): Promise<void> {
  count.value += 1; // no await, so no other task can run in the middle
}

async function addWithAwait(count: Counter): Promise<void> {
  const value = count.value;
  await pause(); // every other task reads the old value here
  count.value = value + 1;
}

console.log("three tasks, each pausing at every await");
// Calling an async function runs it up to its first await, and returns a
// Promise. Promise.all waits for all of them, and keeps the results in order.
const results = await Promise.all([task("a", 2), task("b", 3), task("c", 1)]);
console.log(`results: ${results.join(", ")}`);

console.log("4 tasks add 1 to a shared count");
const atOnce: Counter = { value: 0 };
await Promise.all([1, 2, 3, 4].map(() => addAtOnce(atOnce)));
console.log(`with no await between reading and writing: ${atOnce.value}`);
const withAwait: Counter = { value: 0 };
await Promise.all([1, 2, 3, 4].map(() => addWithAwait(withAwait)));
console.log(`with an await between reading and writing: ${withAwait.value}`);
