use std::error::Error;
use std::fmt;

// Every way a withdraw can fail, with the data each one carries.
#[derive(Debug)]
enum WithdrawError {
    InvalidAmount(i64),
    InsufficientFunds { needed: i64, available: i64 },
    Frozen,
}

impl fmt::Display for WithdrawError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            WithdrawError::InvalidAmount(amount) => write!(f, "invalid amount: {amount}"),
            WithdrawError::InsufficientFunds { needed, available } => {
                write!(f, "insufficient funds: need {needed}, have {available}")
            }
            WithdrawError::Frozen => write!(f, "account is frozen"),
        }
    }
}

impl Error for WithdrawError {}

struct Account {
    balance: i64,
    frozen: bool,
}

impl Account {
    fn withdraw(&mut self, amount: i64) -> Result<(), WithdrawError> {
        if amount <= 0 {
            return Err(WithdrawError::InvalidAmount(amount));
        }
        if self.frozen {
            return Err(WithdrawError::Frozen);
        }
        if amount > self.balance {
            return Err(WithdrawError::InsufficientFunds {
                needed: amount,
                available: self.balance,
            });
        }
        self.balance -= amount;
        Ok(())
    }
}

fn attempt(account: &mut Account, amount: i64) {
    match account.withdraw(amount) {
        Ok(()) => println!("withdraw {amount}: ok, {} left", account.balance),
        // error @ binds the whole error and also pulls out available.
        Err(error @ WithdrawError::InsufficientFunds { available, .. }) => {
            println!("withdraw {amount}: error: {error}");
            account.withdraw(available).expect("the balance covers it");
            println!("  took {available} instead, {} left", account.balance);
        }
        Err(error @ WithdrawError::Frozen) => {
            println!("withdraw {amount}: error: {error}");
            println!("  call the bank to unfreeze it");
        }
        Err(error) => println!("withdraw {amount}: error: {error}"),
    }
}

fn main() {
    let mut account = Account {
        balance: 100,
        frozen: false,
    };
    println!("start with {}", account.balance);
    for amount in [30, -5, 100] {
        attempt(&mut account, amount);
    }
    println!("freeze the account");
    account.frozen = true;
    attempt(&mut account, 10);
}
