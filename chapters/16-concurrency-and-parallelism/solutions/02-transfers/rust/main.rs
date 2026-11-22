use std::io::{self, Read};
use std::sync::Mutex;
use std::thread;

struct Transfer {
    source: usize,
    target: usize,
    amount: i64,
}

fn plural(count: usize, noun: &str) -> String {
    if count == 1 {
        format!("{count} {noun}")
    } else {
        format!("{count} {noun}s")
    }
}

// The whole numbers on a line, or None if a word isn't one.
// Collecting Options into an Option gives None if any of them is None.
fn numbers(line: &str) -> Option<Vec<i64>> {
    let is_number = |word: &str| word.bytes().all(|b| b.is_ascii_digit());
    line.split_whitespace()
        .map(|word| {
            if is_number(word) {
                word.parse().ok()
            } else {
                None
            }
        })
        .collect()
}

// Each balance is inside its own Mutex, so the only way to reach a balance is
// to lock it, and transfers between different accounts run at the same time.
fn transfer(accounts: &[Mutex<i64>], t: &Transfer) {
    if t.source == t.target {
        return; // locking one Mutex twice on a thread deadlocks or panics
    }
    // Every worker takes the two locks in the same order, lower account
    // first, so no two workers can each hold the lock the other wants.
    let (low, high) = (t.source.min(t.target), t.source.max(t.target));
    let mut low_balance = accounts[low].lock().unwrap();
    let mut high_balance = accounts[high].lock().unwrap();
    let (source, target) = if t.source == low {
        (&mut *low_balance, &mut *high_balance)
    } else {
        (&mut *high_balance, &mut *low_balance)
    };
    let (source_balance, target_balance) = (*source, *target);
    // Lets another thread run between the read and the write, as it could
    // while a real bank waits on its database.
    thread::yield_now();
    *source = source_balance - t.amount;
    *target = target_balance + t.amount;
} // the guards are dropped here, which unlocks both

fn parse(input: &str) -> Result<(usize, Vec<i64>, Vec<Transfer>), String> {
    let lines: Vec<&str> = input.lines().collect();
    let workers = match lines.first().and_then(|line| numbers(line)).as_deref() {
        Some(&[n]) if (1..=16).contains(&n) => n as usize,
        _ => return Err("line 1 must be the number of workers, from 1 to 16".to_string()),
    };
    let balances = match lines.get(1).and_then(|line| numbers(line)) {
        Some(balances) if !balances.is_empty() => balances,
        _ => return Err("line 2 must be the starting balances".to_string()),
    };
    let mut transfers = Vec::new();
    for (i, line) in lines.iter().enumerate().skip(2) {
        let number = i + 1;
        match numbers(line).as_deref() {
            Some([]) => {}
            Some(&[source, target, amount]) => {
                let (source, target) = (source as usize, target as usize);
                if let Some(missing) = [source, target].into_iter().find(|&a| a >= balances.len()) {
                    println!("error: line {number}: no account {missing}");
                } else {
                    transfers.push(Transfer {
                        source,
                        target,
                        amount,
                    });
                }
            }
            _ => {
                println!("error: line {number}: a transfer is three numbers: from, to, and amount")
            }
        }
    }
    Ok((workers, balances, transfers))
}

fn main() {
    let mut input = String::new();
    io::stdin()
        .read_to_string(&mut input)
        .expect("can't read stdin");
    let (workers, balances, transfers) = match parse(&input) {
        Ok(parsed) => parsed,
        Err(error) => {
            println!("error: {error}");
            return;
        }
    };
    let accounts: Vec<Mutex<i64>> = balances.into_iter().map(Mutex::new).collect();

    // Worker i applies transfers i, i + workers, i + 2 * workers, and so on.
    // The threads borrow accounts and transfers, and the scope waits for
    // them before it returns.
    thread::scope(|scope| {
        for i in 0..workers {
            let (accounts, transfers) = (&accounts, &transfers);
            scope.spawn(move || {
                for t in transfers.iter().skip(i).step_by(workers) {
                    transfer(accounts, t);
                }
            });
        }
    });

    println!(
        "{}, {}, {}",
        plural(workers, "worker"),
        plural(accounts.len(), "account"),
        plural(transfers.len(), "transfer")
    );
    for i in 0..workers {
        let share: Vec<&Transfer> = transfers.iter().skip(i).step_by(workers).collect();
        let moved: i64 = share.iter().map(|t| t.amount).sum();
        println!(
            "worker {i}: {}, {moved} moved",
            plural(share.len(), "transfer")
        );
    }
    // into_inner takes the value out of a Mutex that nothing else can reach.
    let balances: Vec<i64> = accounts
        .into_iter()
        .map(|a| a.into_inner().unwrap())
        .collect();
    for (i, balance) in balances.iter().enumerate() {
        println!("account {i}: {balance}");
    }
    println!("total: {}", balances.iter().sum::<i64>());
}
