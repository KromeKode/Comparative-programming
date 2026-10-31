// The base class: check for it to handle every kind of withdraw failure.
class WithdrawError extends Error {}

class InvalidAmount extends WithdrawError {
  readonly amount: number;
  constructor(amount: number) {
    super(`invalid amount: ${amount}`);
    this.amount = amount;
  }
}

class InsufficientFunds extends WithdrawError {
  readonly needed: number;
  readonly available: number;
  constructor(needed: number, available: number) {
    super(`insufficient funds: need ${needed}, have ${available}`);
    this.needed = needed;
    this.available = available;
  }
}

class AccountFrozen extends WithdrawError {
  constructor() {
    super("account is frozen");
  }
}

class Account {
  balance: number;
  frozen = false;
  constructor(balance: number) {
    this.balance = balance;
  }

  // Nothing in this signature says it can throw.
  withdraw(amount: number): void {
    if (amount <= 0) {
      throw new InvalidAmount(amount);
    }
    if (this.frozen) {
      throw new AccountFrozen();
    }
    if (amount > this.balance) {
      throw new InsufficientFunds(amount, this.balance);
    }
    this.balance -= amount;
  }
}

function attempt(account: Account, amount: number): void {
  try {
    account.withdraw(amount);
    console.log(`withdraw ${amount}: ok, ${account.balance} left`);
  } catch (error) {
    // error has the type unknown until instanceof narrows it.
    if (!(error instanceof WithdrawError)) {
      throw error;
    }
    console.log(`withdraw ${amount}: error: ${error.message}`);
    if (error instanceof InsufficientFunds) {
      account.withdraw(error.available);
      console.log(`  took ${error.available} instead, ${account.balance} left`);
    } else if (error instanceof AccountFrozen) {
      console.log("  call the bank to unfreeze it");
    }
  }
}

const account = new Account(100);
console.log(`start with ${account.balance}`);
for (const amount of [30, -5, 100]) {
  attempt(account, amount);
}
console.log("freeze the account");
account.frozen = true;
attempt(account, 10);
