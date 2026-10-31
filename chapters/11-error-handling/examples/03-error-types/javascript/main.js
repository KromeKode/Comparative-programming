// The base class: check for it to handle every kind of withdraw failure.
class WithdrawError extends Error {}

class InvalidAmount extends WithdrawError {
  constructor(amount) {
    super(`invalid amount: ${amount}`);
    this.amount = amount;
  }
}

class InsufficientFunds extends WithdrawError {
  constructor(needed, available) {
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
  constructor(balance) {
    this.balance = balance;
    this.frozen = false;
  }

  withdraw(amount) {
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

function attempt(account, amount) {
  try {
    account.withdraw(amount);
    console.log(`withdraw ${amount}: ok, ${account.balance} left`);
  } catch (error) {
    // catch receives anything thrown, so pass on what we do not expect.
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
