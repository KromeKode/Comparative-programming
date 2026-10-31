use std::collections::HashMap;
use std::io::{self, BufRead};

// Returns the pair with the smallest j, and for that j the smallest i.
fn two_sum(target: i64, numbers: &[i64]) -> Option<(usize, usize)> {
    // Each number seen so far, mapped to the first index where it appeared.
    let mut first_index: HashMap<i64, usize> = HashMap::new();
    for (j, &n) in numbers.iter().enumerate() {
        if let Some(&i) = first_index.get(&(target - n)) {
            return Some((i, j));
        }
        first_index.entry(n).or_insert(j); // keeps the first index
    }
    None
}

fn main() {
    for line in io::stdin().lock().lines() {
        let line = line.expect("failed to read stdin");
        let Some((target, rest)) = line.split_once(':') else {
            continue; // a blank line
        };
        let target: i64 = target.trim().parse().expect("not a number");
        let numbers: Vec<i64> = rest
            .split_whitespace()
            .map(|word| word.parse().expect("not a number"))
            .collect();
        match two_sum(target, &numbers) {
            Some((i, j)) => println!("{i} {j}"),
            None => println!("none"),
        }
    }
}
