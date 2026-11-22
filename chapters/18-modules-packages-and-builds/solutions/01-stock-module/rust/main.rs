// Exercise 18.1: a stock module. This file is given: write stock.rs so that it
// builds. It reads the commands, and leaves keeping count to the module.
mod stock;

use std::io::{self, Read};
use stock::Stock;

/// An amount is a whole number from 1 to 999,999,999,999,999: digits, with any
/// zeros in front, and at most 15 more after them.
fn parse_amount(word: &str) -> Option<u64> {
    let digits = word.trim_start_matches('0');
    if digits.is_empty() || digits.len() > 15 || !word.bytes().all(|b| b.is_ascii_digit()) {
        return None;
    }
    digits.parse().ok()
}

fn main() {
    let mut input = String::new();
    io::stdin()
        .read_to_string(&mut input)
        .expect("can't read stdin");
    let mut stock = Stock::new();
    for (number, line) in (1..).zip(input.lines()) {
        let words: Vec<&str> = line.split_whitespace().collect();
        match words[..] {
            [] => {}
            ["add", item, amount] if parse_amount(amount).is_some() => {
                let amount = parse_amount(amount).unwrap();
                stock.add(item, amount);
                println!("added {amount} {item}, now {}", stock.count(item));
            }
            ["take", item, amount] if parse_amount(amount).is_some() => {
                let amount = parse_amount(amount).unwrap();
                if stock.take(item, amount) {
                    println!("took {amount} {item}, now {}", stock.count(item));
                } else {
                    println!(
                        "error: line {number}: only {} {item} in stock",
                        stock.count(item)
                    );
                }
            }
            ["count", item] => println!("{item}: {}", stock.count(item)),
            ["list"] => {
                let items: Vec<String> = stock
                    .items()
                    .iter()
                    .map(|(item, count)| format!("{item} {count}"))
                    .collect();
                let text = if items.is_empty() {
                    "empty".to_string()
                } else {
                    items.join(", ")
                };
                println!("stock: {text}");
            }
            _ => println!("error: line {number}: expected add, take, count, or list"),
        }
    }
}
