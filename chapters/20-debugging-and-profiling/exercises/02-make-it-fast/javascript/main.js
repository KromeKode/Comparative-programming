// Simulates a day of payments between bank accounts, and prints a summary. It gives
// the right answers, but slowly: ../README.md says what it prints, and how fast it
// has to be.
import { readFileSync } from "node:fs";

const START = 1000; // what each account starts with

// The Park-Miller generator, which every language's version uses, so that they all
// make up the same transfers from the same seed.
class Random {
  constructor(seed) {
    this.state = seed;
  }

  next() {
    this.state = (this.state * 48271) % 2147483647;
    return this.state;
  }
}

function accountName(i) {
  return `acct${String(i).padStart(6, "0")}`;
}

// Gives the account with this name.
function find(accounts, name) {
  for (const account of accounts) {
    if (account.name === name) return account;
  }
  throw new Error(`no account called ${name}`);
}

function total(accounts) {
  let sum = 0;
  for (const account of accounts) sum += account.balance;
  return sum;
}

const [count, transfers, seed] = readFileSync(0, "utf8")
  .trim()
  .split(/\s+/)
  .map(Number);
const accounts = [];
for (let i = 0; i < count; i++)
  accounts.push({ name: accountName(i), balance: START });

const random = new Random(seed);
let refused = 0;
for (let t = 0; t < transfers; t++) {
  const source = find(accounts, accountName(random.next() % count));
  const target = find(accounts, accountName(random.next() % count));
  const amount = (random.next() % 500) + 1;
  if (source.balance < amount) {
    refused++;
  } else {
    source.balance -= amount;
    target.balance += amount;
  }
  // Money is never made or lost, so the total stays the same.
  if (total(accounts) !== count * START) {
    console.error("error: the total has changed");
    process.exit(1);
  }
}

console.log(`transfers: ${transfers}, refused: ${refused}`);
console.log(`total: ${total(accounts)}`);
accounts.sort((a, b) => b.balance - a.balance || (a.name < b.name ? -1 : 1));
for (const account of accounts.slice(0, 5)) {
  console.log(`${account.name} ${account.balance}`);
}
