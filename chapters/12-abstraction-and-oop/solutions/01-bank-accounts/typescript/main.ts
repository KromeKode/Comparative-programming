import { createInterface } from "node:readline";

const OVERDRAFT_LIMIT = 100;
const OVERDRAFT_FEE = 5;
const SAVINGS_WITHDRAWALS = 2;

class AccountError extends Error {}

// What every account has. An abstract class cannot be made with new, and each
// kind must fill in the abstract methods.
abstract class Account {
  readonly name: string;
  protected balanceValue = 0;

  constructor(name: string) {
    this.name = name;
  }

  get balance(): number {
    return this.balanceValue;
  }

  deposit(amount: number): void {
    this.balanceValue += amount;
  }

  withdraw(amount: number): void {
    this.checkWithdrawal(amount);
    this.balanceValue -= amount;
  }

  protected abstract checkWithdrawal(amount: number): void;

  abstract monthEnd(): string;
}

class Checking extends Account {
  protected override checkWithdrawal(amount: number): void {
    if (this.balanceValue - amount < -OVERDRAFT_LIMIT) {
      throw new AccountError(`over the overdraft limit of ${OVERDRAFT_LIMIT}`);
    }
  }

  override monthEnd(): string {
    if (this.balanceValue < 0) {
      this.balanceValue -= OVERDRAFT_FEE;
      return `fee ${OVERDRAFT_FEE}`;
    }
    return "no fee";
  }
}

class Savings extends Account {
  private withdrawals = 0;

  protected override checkWithdrawal(amount: number): void {
    if (this.withdrawals === SAVINGS_WITHDRAWALS) {
      throw new AccountError(`only ${SAVINGS_WITHDRAWALS} withdrawals a month`);
    }
    if (amount > this.balanceValue) {
      throw new AccountError("insufficient funds");
    }
  }

  override withdraw(amount: number): void {
    super.withdraw(amount);
    this.withdrawals++;
  }

  override monthEnd(): string {
    const interest = Math.floor(this.balanceValue / 100);
    this.balanceValue += interest;
    this.withdrawals = 0;
    return `interest ${interest}`;
  }
}

// The type of a class that can be made with new: here, any kind of account.
const KINDS = new Map<string, new (name: string) => Account>([
  ["checking", Checking],
  ["savings", Savings],
]);

function find(accounts: Map<string, Account>, name: string): Account {
  const account = accounts.get(name);
  if (account === undefined) {
    throw new AccountError(`no account named ${name}`);
  }
  return account;
}

function run(accounts: Map<string, Account>, words: string[]): void {
  const [command, ...args] = words;
  if (command === "open" && args.length === 2) {
    const [kind, name] = args as [string, string];
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
    const account = find(accounts, args[0]!);
    const amount = Number(args[1]);
    account.deposit(amount);
    console.log(
      `${account.name}: deposited ${amount}, balance ${account.balance}`,
    );
  } else if (command === "withdraw" && args.length === 2) {
    const account = find(accounts, args[0]!);
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
    for (const account of accounts.values()) {
      const change = account.monthEnd();
      console.log(`${account.name}: ${change}, balance ${account.balance}`);
    }
  }
}

const accounts = new Map<string, Account>();
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
