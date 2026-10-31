use std::io::{self, BufRead};

// Rust will not hand out two &mut into one Vec at the same time, so
// `mem::swap(&mut words[i], &mut words[j])` does not compile. Swap takes the
// slice and two indexes instead, and split_at_mut gives two &mut that the
// compiler can see do not overlap.
fn swap(words: &mut [String], i: usize, j: usize) {
    if i == j {
        return;
    }
    let (low, high) = (i.min(j), i.max(j));
    let (left, right) = words.split_at_mut(high);
    std::mem::swap(&mut left[low], &mut right[0]);
}

// Reverses words[from..=to] and returns how many swaps that took.
fn reverse(words: &mut [String], mut from: usize, mut to: usize) -> u32 {
    let mut swaps = 0;
    while from < to {
        swap(words, from, to);
        swaps += 1;
        from += 1;
        to -= 1;
    }
    swaps
}

fn parse_position(text: &str, count: usize) -> Option<usize> {
    if !text.bytes().all(|b| b.is_ascii_digit()) {
        println!("error: \"{text}\" is not a position");
        return None;
    }
    // parse fails only when the number is too big for usize: out of range.
    match text.parse::<usize>() {
        Ok(value) if value >= 1 && value <= count => Some(value),
        _ => {
            println!("error: position {text} is out of range (1 to {count})");
            None
        }
    }
}

fn main() {
    let mut lines = io::stdin()
        .lock()
        .lines()
        .map(|line| line.expect("failed to read stdin"));
    let mut words: Vec<String> = match lines.next() {
        Some(first) => first.split_whitespace().map(String::from).collect(),
        None => Vec::new(),
    };

    let mut swaps = 0;
    for line in lines {
        let parts: Vec<&str> = line.split_whitespace().collect();
        let Some(&command) = parts.first() else {
            continue;
        };
        if command != "swap" && command != "reverse" {
            println!("error: unknown command \"{command}\"");
            continue;
        }
        if parts.len() != 3 {
            let got = parts.len() - 1;
            println!("error: {command} needs 2 positions, got {got}");
            continue;
        }
        let Some(i) = parse_position(parts[1], words.len()) else {
            continue;
        };
        let Some(j) = parse_position(parts[2], words.len()) else {
            continue;
        };
        if command == "swap" {
            swap(&mut words, i - 1, j - 1);
            swaps += 1;
        } else {
            swaps += reverse(&mut words, i.min(j) - 1, i.max(j) - 1);
        }
        println!("{}", words.join(" "));
    }
    println!("swaps: {swaps}");
}
