use std::io::{self, BufRead};

fn main() {
    for line in io::stdin().lock().lines() {
        let line = line.expect("failed to read stdin");
        let mut parts = line.split_whitespace();
        let (Some(number), Some(unit)) = (parts.next(), parts.next()) else {
            continue; // a blank line
        };
        let value: f64 = number.parse().expect("not a number");
        if unit == "C" {
            println!("{value:.1} C = {:.1} F", value * 9.0 / 5.0 + 32.0);
        } else {
            println!("{value:.1} F = {:.1} C", (value - 32.0) * 5.0 / 9.0);
        }
    }
}
