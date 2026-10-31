use std::io::{self, BufRead};

const OVERDRAFT_LIMIT: i64 = 100;
const OVERDRAFT_FEE: i64 = 5;
const SAVINGS_WITHDRAWALS: u32 = 2;

// Rust has no inheritance, so an Account holds the parts every account shares
// and a Rules value for the part that differs between kinds.
trait Rules {
    fn check_withdrawal(&self, balance: i64, amount: i64) -> Result<(), String>;

    // Called after each withdrawal that worked. Most kinds do nothing.
    fn after_withdrawal(&mut self) {}

    // Changes the balance as the kind requires, and says what it did.
    fn month_end(&mut self, balance: &mut i64) -> String;
}

struct Checking;

impl Rules for Checking {
    fn check_withdrawal(&self, balance: i64, amount: i64) -> Result<(), String> {
        if balance - amount < -OVERDRAFT_LIMIT {
            return Err(format!("over the overdraft limit of {OVERDRAFT_LIMIT}"));
        }
        Ok(())
    }

    fn month_end(&mut self, balance: &mut i64) -> String {
        if *balance < 0 {
            *balance -= OVERDRAFT_FEE;
            return format!("fee {OVERDRAFT_FEE}");
        }
        "no fee".to_string()
    }
}

struct Savings {
    withdrawals: u32,
}

impl Rules for Savings {
    fn check_withdrawal(&self, balance: i64, amount: i64) -> Result<(), String> {
        if self.withdrawals == SAVINGS_WITHDRAWALS {
            return Err(format!("only {SAVINGS_WITHDRAWALS} withdrawals a month"));
        }
        if amount > balance {
            return Err("insufficient funds".to_string());
        }
        Ok(())
    }

    fn after_withdrawal(&mut self) {
        self.withdrawals += 1;
    }

    fn month_end(&mut self, balance: &mut i64) -> String {
        let interest = *balance / 100;
        *balance += interest;
        self.withdrawals = 0;
        format!("interest {interest}")
    }
}

struct Account {
    name: String,
    balance: i64,
    rules: Box<dyn Rules>,
}

impl Account {
    fn deposit(&mut self, amount: i64) {
        self.balance += amount;
    }

    fn withdraw(&mut self, amount: i64) -> Result<(), String> {
        self.rules.check_withdrawal(self.balance, amount)?;
        self.balance -= amount;
        self.rules.after_withdrawal();
        Ok(())
    }

    fn month_end(&mut self) -> String {
        self.rules.month_end(&mut self.balance)
    }
}

fn make_rules(kind: &str) -> Option<Box<dyn Rules>> {
    match kind {
        "checking" => Some(Box::new(Checking)),
        "savings" => Some(Box::new(Savings { withdrawals: 0 })),
        _ => None,
    }
}

fn find<'a>(accounts: &'a mut [Account], name: &str) -> Result<&'a mut Account, String> {
    accounts
        .iter_mut()
        .find(|account| account.name == name)
        .ok_or(format!("no account named {name}"))
}

fn parse_amount(text: &str) -> Result<i64, String> {
    text.parse()
        .map_err(|_| format!("not a whole number: {text}"))
}

fn run(accounts: &mut Vec<Account>, words: &[&str]) -> Result<(), String> {
    match words {
        ["open", kind, name] => {
            let rules = make_rules(kind).ok_or(format!("unknown account kind: {kind}"))?;
            if accounts.iter().any(|account| account.name == *name) {
                return Err(format!("{name} already has an account"));
            }
            accounts.push(Account {
                name: name.to_string(),
                balance: 0,
                rules,
            });
            println!("{name}: opened a {kind} account");
        }
        ["deposit", name, amount] => {
            let amount = parse_amount(amount)?;
            let account = find(accounts, name)?;
            account.deposit(amount);
            println!("{name}: deposited {amount}, balance {}", account.balance);
        }
        ["withdraw", name, amount] => {
            let amount = parse_amount(amount)?;
            let account = find(accounts, name)?;
            account
                .withdraw(amount)
                .map_err(|error| format!("{name}: {error}"))?;
            println!("{name}: withdrew {amount}, balance {}", account.balance);
        }
        ["month"] => {
            for account in accounts {
                let change = account.month_end();
                println!("{}: {change}, balance {}", account.name, account.balance);
            }
        }
        _ => {}
    }
    Ok(())
}

fn main() {
    let mut accounts = Vec::new();
    for line in io::stdin().lock().lines() {
        let line = line.expect("stdin should be readable");
        let words: Vec<&str> = line.split_whitespace().collect();
        if let Err(error) = run(&mut accounts, &words) {
            println!("error: {error}");
        }
    }
}
