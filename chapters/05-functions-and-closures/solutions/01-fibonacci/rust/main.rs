use std::collections::HashMap;
use std::io::{self, BufRead};

// A mutable global in Rust needs a lock, thread_local!, or unsafe, and a closure
// cannot easily call itself, so the memo is passed in explicitly.
fn fib(n: u32, memo: &mut HashMap<u32, u64>) -> u64 {
    if n < 2 {
        return u64::from(n);
    }
    if let Some(&value) = memo.get(&n) {
        return value;
    }
    let value = fib(n - 1, memo) + fib(n - 2, memo);
    memo.insert(n, value);
    value
}

fn main() {
    let mut memo = HashMap::new();
    for line in io::stdin().lock().lines() {
        let line = line.expect("failed to read stdin");
        let text = line.trim();
        if text.is_empty() {
            continue;
        }
        let n: u32 = text.parse().expect("n must be a whole number from 0 to 92");
        println!("fib({n}) = {}", fib(n, &mut memo));
    }
}
