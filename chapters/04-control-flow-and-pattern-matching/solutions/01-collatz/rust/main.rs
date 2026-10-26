use std::io::{self, BufRead};

fn main() {
    for line in io::stdin().lock().lines() {
        let line = line.expect("failed to read stdin");
        let text = line.trim();
        if text.is_empty() {
            continue;
        }
        let start: u64 = text.parse().expect("not a positive whole number");
        // The values climb far past 2^32 for some starts, so use 64 bits.
        let mut n = start;
        let mut peak = n;
        let mut steps = 0;
        while n != 1 {
            n = if n % 2 == 0 { n / 2 } else { 3 * n + 1 };
            peak = peak.max(n);
            steps += 1;
        }
        println!("{start}: steps {steps}, peak {peak}");
    }
}
