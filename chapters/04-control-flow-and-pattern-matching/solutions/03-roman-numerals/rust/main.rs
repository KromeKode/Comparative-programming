use std::io::{self, BufRead};

const TABLE: [(u32, &str); 13] = [
    (1000, "M"),
    (900, "CM"),
    (500, "D"),
    (400, "CD"),
    (100, "C"),
    (90, "XC"),
    (50, "L"),
    (40, "XL"),
    (10, "X"),
    (9, "IX"),
    (5, "V"),
    (4, "IV"),
    (1, "I"),
];

fn to_roman(mut n: u32) -> String {
    let mut out = String::new();
    for (value, symbol) in TABLE {
        while n >= value {
            out.push_str(symbol); // take the biggest value that still fits
            n -= value;
        }
    }
    out
}

fn letter_value(letter: char) -> i32 {
    match letter {
        'I' => 1,
        'V' => 5,
        'X' => 10,
        'L' => 50,
        'C' => 100,
        'D' => 500,
        'M' => 1000,
        _ => panic!("not a Roman numeral letter: {letter}"),
    }
}

fn from_roman(text: &str) -> i32 {
    let values: Vec<i32> = text.chars().map(letter_value).collect();
    let mut total = 0;
    for (i, &value) in values.iter().enumerate() {
        // A let chain (Rust 1.88, 2024 edition): if there is a next letter AND it is bigger.
        if let Some(&next) = values.get(i + 1)
            && value < next
        {
            total -= value; // a smaller letter before a bigger one: IV is 5 - 1
        } else {
            total += value;
        }
    }
    total
}

fn main() {
    for line in io::stdin().lock().lines() {
        let line = line.expect("failed to read stdin");
        let text = line.trim();
        if text.is_empty() {
            continue;
        }
        match text.parse::<u32>() {
            Ok(n) => println!("{n} -> {}", to_roman(n)),
            Err(_) => println!("{text} -> {}", from_roman(text)),
        }
    }
}
