import { createInterface } from "node:readline";

const OVERDRAFT_LIMIT = 100;
const OVERDRAFT_FEE = 5;
const SAVINGS_WITHDRAWALS = 2;

class AccountError extends Error {}

// What every account has. Each kind fills in its own rules.
class Account {
  constructor(name) {
    this.name = name;
    this.balance = 0;
  }

  deposit(amount) {
    this.balance += amount;
  }

  withdraw(amount) {
    this.checkWithdrawal(amount);
    this.balance -= amount;
  }

  // JavaScript has no abstract methods, so these throw if a kind forgets one.
  checkWithdrawal(amount) {
    throw new Error(`${this.constructor.name} must define checkWithdrawal`);
  }

  monthEnd() {
    throw new Error(`${this.constructor.name} must define monthEnd`);
  }
}

class Checking extends Account {
  checkWithdrawal(amount) {
    if (this.balance - amount < -OVERDRAFT_LIMIT) {
      throw new AccountError(`over the overdraft limit of ${OVERDRAFT_LIMIT}`);
    }
  }

  monthEnd() {
    if (this.balance < 0) {
      this.balance -= OVERDRAFT_FEE;
      return `fee ${OVERDRAFT_FEE}`;
    }
    return "no fee";
  }
}

class Savings extends Account {
  withdrawals = 0;

  checkWithdrawal(amount) {
    if (this.withdrawals === SAVINGS_WITHDRAWALS) {
      throw new AccountError(`only ${SAVINGS_WITHDRAWALS} withdrawals a month`);
    }
    if (amount > this.balance) {
      throw new AccountError("insufficient funds");
    }
  }

  withdraw(amount) {
    super.withdraw(amount);
    this.withdrawals++;
  }

  monthEnd() {
    const interest = Math.floor(this.balance / 100);
    this.balance += interest;
    this.withdrawals = 0;
    return `interest ${interest}`;
  }
}

const KINDS = new Map([
  ["checking", Checking],
  ["savings", Savings],
]);

function find(accounts, name) {
  const account = accounts.get(name);
  if (account === undefined) {
    throw new AccountError(`no account named ${name}`);
  }
  return account;
}

function run(accounts, words) {
  const [command, ...args] = words;
  if (command === "open" && args.length === 2) {
    const [kind, name] = args;
    const Kind = KINDS.get(kind);
    if (Kind === undefined) {
      throw new AccountError(`unknown account kind: ${kind}`);
    }
    if (accounts.has(name)) {
      throw new AccountError(`${name} already has an account`);
    }
    accounts.set(name, new Kind(name));
    console.log(`${name}: opened a ${kind} account`);
  } else if (command === "deposit" && args.length === 2) {
    const account = find(accounts, args[0]);
    const amount = Number(args[1]);
    account.deposit(amount);
    console.log(
      `${account.name}: deposited ${amount}, balance ${account.balance}`,
    );
  } else if (command === "withdraw" && args.length === 2) {
    const account = find(accounts, args[0]);
    const amount = Number(args[1]);
    try {
      account.withdraw(amount);
    } catch (error) {
      if (error instanceof AccountError) {
        throw new AccountError(`${account.name}: ${error.message}`);
      }
      throw error;
    }
    console.log(
      `${account.name}: withdrew ${amount}, balance ${account.balance}`,
    );
  } else if (command === "month" && args.length === 0) {
    // A Map keeps its keys in the order they were added.
    for (const account of accounts.values()) {
      const change = account.monthEnd();
      console.log(`${account.name}: ${change}, balance ${account.balance}`);
    }
  }
}

const accounts = new Map();
for await (const line of createInterface({ input: process.stdin })) {
  const words = line.split(/\s+/).filter((word) => word !== "");
  try {
    run(accounts, words);
  } catch (error) {
    if (!(error instanceof AccountError)) {
      throw error;
    }
    console.log(`error: ${error.message}`);
  }
}
