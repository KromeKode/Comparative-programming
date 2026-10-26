use std::io::{self, BufRead};

fn main() {
    for line in io::stdin().lock().lines() {
        let line = line.expect("failed to read stdin");
        let numbers: Vec<i32> = line
            .split_whitespace()
            .map(|word| word.parse().expect("not a 32-bit whole number"))
            .collect();
        let [a, b] = numbers[..] else {
            continue; // a blank line
        };
        // Rust's / and % truncate toward zero.
        let q = a / b;
        let r = a % b;
        // div_euclid and rem_euclid are not floored division: they differ when b < 0.
        // So adjust by hand, as in C.
        let (fq, fm) = if r != 0 && (r < 0) != (b < 0) {
            (q - 1, r + b)
        } else {
            (q, r)
        };
        println!("{a} {b} -> truncated {q} {r}, floored {fq} {fm}");
    }
}
