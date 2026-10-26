use std::io::{self, BufRead};

fn main() {
    for line in io::stdin().lock().lines() {
        let line = line.expect("failed to read stdin");
        let text = line.trim();
        if text.is_empty() {
            continue;
        }
        let n: u32 = text.parse().expect("not a 32-bit unsigned number");
        let ones = n.count_ones();
        let width = u32::BITS - n.leading_zeros(); // Rust 1.97 also has n.bit_width()
        println!("{n}: {n:b} (ones {ones}, width {width})");
    }
}
