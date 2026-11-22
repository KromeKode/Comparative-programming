import { readFileSync } from "node:fs";

interface Transfer {
  source: number;
  target: number;
  amount: number;
}

function plural(count: number, noun: string): string {
  return `${count} ${noun}${count === 1 ? "" : "s"}`;
}

// The whole numbers on a line, or null if a word isn't one.
function numbers(line: string): number[] | null {
  const words = line.split(/\s+/).filter((word) => word !== "");
  return words.every((word) => /^[0-9]+$/.test(word))
    ? words.map(Number)
    : null;
}

// An async lock. acquire returns a promise of a function that releases the
// lock, and the promise resolves once every earlier holder has released it.
class Lock {
  #last: Promise<void> = Promise.resolve();

  acquire(): Promise<() => void> {
    let release: () => void = () => {};
    const released = new Promise<void>((resolve) => (release = resolve));
    const acquired = this.#last.then(() => release);
    this.#last = released;
    return acquired;
  }
}

// Lets the other tasks run, as a real bank would while it waits on its
// database. setImmediate runs its callback on the next turn of the event loop.
const pause = (): Promise<void> =>
  new Promise((resolve) => setImmediate(resolve));

class Bank {
  readonly balances: number[];
  // One lock for each account, so that transfers between different
  // accounts can wait on the database at the same time.
  readonly locks: Lock[];

  constructor(balances: number[]) {
    this.balances = balances;
    this.locks = balances.map(() => new Lock());
  }

  async transfer({ source, target, amount }: Transfer): Promise<void> {
    if (source === target) return; // a task would wait for itself forever
    // Every task takes the two locks in the same order, lower account
    // first, so no two tasks can each hold the lock the other wants.
    const releaseFirst = await this.locks[Math.min(source, target)].acquire();
    const releaseSecond = await this.locks[Math.max(source, target)].acquire();
    try {
      const sourceBalance = this.balances[source];
      const targetBalance = this.balances[target];
      await pause();
      this.balances[source] = sourceBalance - amount;
      this.balances[target] = targetBalance + amount;
    } finally {
      releaseSecond();
      releaseFirst();
    }
  }
}

interface Parsed {
  workers: number;
  balances: number[];
  transfers: Transfer[];
}

function parse(lines: string[]): Parsed {
  const first = numbers(lines[0] ?? "");
  if (first?.length !== 1 || first[0] < 1 || first[0] > 16) {
    throw new Error("line 1 must be the number of workers, from 1 to 16");
  }
  const balances = numbers(lines[1] ?? "");
  if (!balances?.length)
    throw new Error("line 2 must be the starting balances");
  const transfers: Transfer[] = [];
  for (let i = 2; i < lines.length; i++) {
    const words = numbers(lines[i]);
    if (words?.length === 0) continue;
    if (words?.length !== 3) {
      console.log(
        `error: line ${i + 1}: a transfer is three numbers: from, to, and amount`,
      );
      continue;
    }
    const [source, target, amount] = words;
    const missing = [source, target].find(
      (account) => account >= balances.length,
    );
    if (missing !== undefined) {
      console.log(`error: line ${i + 1}: no account ${missing}`);
      continue;
    }
    transfers.push({ source, target, amount });
  }
  return { workers: first[0], balances, transfers };
}

const lines = readFileSync(0, "utf8").split("\n");
if (lines.at(-1) === "") lines.pop(); // after the newline that ends the input
let parsed: Parsed;
try {
  parsed = parse(lines);
} catch (error) {
  console.log(`error: ${(error as Error).message}`);
  process.exit(0);
}
const { workers, balances, transfers } = parsed;
const bank = new Bank(balances);

// Worker i applies transfers i, i + workers, i + 2 * workers, and so on. The
// workers are async functions on this one thread, and each one stops at every
// await, so the others run while it waits.
const shares = Array.from({ length: workers }, (_, i) =>
  transfers.filter((_, j) => j % workers === i),
);
await Promise.all(
  shares.map(async (share) => {
    for (const transfer of share) await bank.transfer(transfer);
  }),
);

console.log(
  `${plural(workers, "worker")}, ${plural(balances.length, "account")}, ` +
    plural(transfers.length, "transfer"),
);
for (const [i, share] of shares.entries()) {
  const moved = share.reduce((sum, t) => sum + t.amount, 0);
  console.log(
    `worker ${i}: ${plural(share.length, "transfer")}, ${moved} moved`,
  );
}
for (const [i, balance] of bank.balances.entries())
  console.log(`account ${i}: ${balance}`);
console.log(`total: ${bank.balances.reduce((a, b) => a + b, 0)}`);
