use std::io::{self, BufRead};

fn main() {
    let mut total: i64 = 0;
    for line in io::stdin().lock().lines() {
        let line = line.expect("failed to read stdin");
        let line = line.trim();
        if line.is_empty() {
            continue;
        }
        let value: i64 = line.parse().expect("each line must be a whole number");
        total += value;
    }
    println!("{total}");
}
