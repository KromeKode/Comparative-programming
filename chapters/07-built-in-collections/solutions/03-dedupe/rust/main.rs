use std::collections::HashSet;
use std::io::{self, BufRead};

fn main() {
    for line in io::stdin().lock().lines() {
        let line = line.expect("failed to read stdin");
        let mut seen = HashSet::new();
        // insert returns false if the value was already in the set.
        let kept: Vec<&str> = line
            .split_whitespace()
            .filter(|word| seen.insert(*word))
            .collect();
        if !kept.is_empty() {
            println!("{}", kept.join(" "));
        }
    }
}
