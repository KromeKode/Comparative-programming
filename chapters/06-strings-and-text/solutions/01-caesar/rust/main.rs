use std::io::{self, BufRead};

// Shifts an ASCII letter and leaves every other char alone.
fn shift_letter(c: char, shift: u8) -> char {
    match c {
        'a'..='z' => ((c as u8 - b'a' + shift) % 26 + b'a') as char,
        'A'..='Z' => ((c as u8 - b'A' + shift) % 26 + b'A') as char,
        _ => c,
    }
}

fn main() {
    for line in io::stdin().lock().lines() {
        let line = line.expect("failed to read stdin");
        let line = line.trim();
        if line.is_empty() {
            continue;
        }
        // split_once splits at the first space only, so the text keeps its own spaces.
        let (number, text) = line.split_once(' ').unwrap_or((line, ""));
        let shift: i64 = number.parse().expect("not a number");
        let shift = shift.rem_euclid(26) as u8; // 0 to 25, even for negative shifts
        let shifted: String = text.chars().map(|c| shift_letter(c, shift)).collect();
        println!("{shifted}");
    }
}
