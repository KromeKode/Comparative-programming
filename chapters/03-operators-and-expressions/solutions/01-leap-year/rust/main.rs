use std::io::{self, BufRead};

fn is_leap(year: u32) -> bool {
    year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)
}

fn main() {
    for line in io::stdin().lock().lines() {
        let line = line.expect("failed to read stdin");
        let text = line.trim();
        if text.is_empty() {
            continue;
        }
        let year: u32 = text.parse().expect("not a year");
        let kind = if is_leap(year) {
            "leap year"
        } else {
            "common year"
        };
        println!("{year}: {kind}");
    }
}
