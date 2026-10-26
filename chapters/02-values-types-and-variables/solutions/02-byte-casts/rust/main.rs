use std::io::{self, BufRead};

fn main() {
    for line in io::stdin().lock().lines() {
        let line = line.expect("failed to read stdin");
        let line = line.trim();
        if line.is_empty() {
            continue;
        }
        let x: i64 = line.parse().expect("not a whole number");
        // `as` between integer types keeps the low bits: it wraps, never panics.
        println!("{x} -> u8 {}, i8 {}", x as u8, x as i8);
    }
}
