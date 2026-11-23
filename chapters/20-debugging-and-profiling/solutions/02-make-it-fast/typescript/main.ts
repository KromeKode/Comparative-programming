// Simulates a day of payments between bank accounts, and prints a summary.
// ../README.md says what it prints, and how fast it has to be.
import { readFileSync } from "node:fs";

const START = 1000; // what each account starts with

// The Park-Miller generator, which every language's version uses, so that they all
// make up the same transfers from the same seed.
class Random {
  private state: number;

  constructor(seed: number) {
    this.state = seed;
  }

  next(): number {
    this.state = (this.state * 48271) % 2147483647;
    return this.state;
  }
}

interface Account {
  name: string;
  balance: number;
}

function accountName(i: number): string {
  return `acct${String(i).padStart(6, "0")}`;
}

function total(accounts: Account[]): number {
  let sum = 0;
  for (const account of accounts) sum += account.balance;
  return sum;
}

const [count = 0, transfers = 0, seed = 1] = readFileSync(0, "utf8")
  .trim()
  .split(/\s+/)
  .map(Number);
const accounts: Account[] = [];
for (let i = 0; i < count; i++)
  accounts.push({ name: accountName(i), balance: START });
// Finding an account by its name was the slowest part: a Map finds it at once, where
// looking through the array took time in proportion to the accounts.
const byName = new Map(accounts.map((account) => [account.name, account]));

const random = new Random(seed);
let refused = 0;
for (let t = 0; t < transfers; t++) {
  const source = byName.get(accountName(random.next() % count))!;
  const target = byName.get(accountName(random.next() % count))!;
  const amount = (random.next() % 500) + 1;
  if (source.balance < amount) {
    refused++;
  } else {
    source.balance -= amount;
    target.balance += amount;
  }
}
// Adding up every account after every transfer was the other slow part. A transfer
// moves money and can't change the total, so checking it once, at the end, catches
// the same bugs.
if (total(accounts) !== count * START) {
  console.error("error: the total has changed");
  process.exit(1);
}

console.log(`transfers: ${transfers}, refused: ${refused}`);
console.log(`total: ${total(accounts)}`);
accounts.sort((a, b) => b.balance - a.balance || (a.name < b.name ? -1 : 1));
for (const account of accounts.slice(0, 5)) {
  console.log(`${account.name} ${account.balance}`);
}
