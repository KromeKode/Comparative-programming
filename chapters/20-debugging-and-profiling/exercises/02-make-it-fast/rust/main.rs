// Simulates a day of payments between bank accounts, and prints a summary. It gives
// the right answers, but slowly: ../README.md says what it prints, and how fast it
// has to be.
use std::io::Read;

const START: i64 = 1000; // what each account starts with

/// The Park-Miller generator, which every language's version uses, so that they all
/// make up the same transfers from the same seed.
struct Random {
    state: i64,
}

impl Random {
    fn next(&mut self) -> i64 {
        self.state = self.state * 48271 % 2147483647;
        self.state
    }
}

struct Account {
    name: String,
    balance: i64,
}

fn account_name(i: i64) -> String {
    format!("acct{i:06}")
}

/// Gives the index of the account with this name.
fn find(accounts: &[Account], name: &str) -> usize {
    accounts
        .iter()
        .position(|account| account.name == name)
        .unwrap()
}

fn total(accounts: &[Account]) -> i64 {
    accounts.iter().map(|account| account.balance).sum()
}

fn main() {
    let mut input = String::new();
    std::io::stdin().read_to_string(&mut input).unwrap();
    let numbers: Vec<i64> = input
        .split_whitespace()
        .map(|n| n.parse().unwrap())
        .collect();
    let (count, transfers, seed) = (numbers[0], numbers[1], numbers[2]);
    let mut accounts: Vec<Account> = (0..count)
        .map(|i| Account {
            name: account_name(i),
            balance: START,
        })
        .collect();

    let mut random = Random { state: seed };
    let mut refused = 0;
    for _ in 0..transfers {
        let source = find(&accounts, &account_name(random.next() % count));
        let target = find(&accounts, &account_name(random.next() % count));
        let amount = random.next() % 500 + 1;
        if accounts[source].balance < amount {
            refused += 1;
        } else {
            accounts[source].balance -= amount;
            accounts[target].balance += amount;
        }
        // Money is never made or lost, so the total stays the same.
        if total(&accounts) != count * START {
            eprintln!("error: the total has changed");
            std::process::exit(1);
        }
    }

    println!("transfers: {transfers}, refused: {refused}");
    println!("total: {}", total(&accounts));
    accounts.sort_by(|a, b| b.balance.cmp(&a.balance).then_with(|| a.name.cmp(&b.name)));
    for account in accounts.iter().take(5) {
        println!("{} {}", account.name, account.balance);
    }
}
