use std::io::{self, BufRead};

fn main() {
    for line in io::stdin().lock().lines() {
        let line = line.expect("failed to read stdin");
        let numbers: Vec<i64> = line
            .split_whitespace()
            .map(|word| word.parse().expect("not a 64-bit whole number"))
            .collect();
        let [a, b] = numbers[..] else {
            continue; // a blank line
        };
        match a.checked_add(b) {
            Some(sum) => println!("{sum}"),
            None => println!("overflow"),
        }
    }
}
