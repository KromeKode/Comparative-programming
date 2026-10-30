use std::cmp::Ordering;
use std::io::{self, BufRead};

fn guesses_text(count: u32) -> String {
    if count == 1 {
        "1 guess".to_string()
    } else {
        format!("{count} guesses")
    }
}

fn main() {
    let mut lines = io::stdin()
        .lock()
        .lines()
        .map(|line| line.expect("failed to read stdin"));
    let secret: i64 = lines
        .next()
        .expect("missing the secret number")
        .trim()
        .parse()
        .expect("the secret must be a whole number");

    let mut guesses = 0;
    for line in lines {
        let text = line.trim();
        match text {
            "" => continue, // a blank line
            // An `if let` guard (Rust 1.95): this arm matches only if the parse succeeds.
            _ if let Ok(guess) = text.parse::<i64>() => {
                guesses += 1;
                match guess.cmp(&secret) {
                    Ordering::Less => println!("{guess}: too low"),
                    Ordering::Greater => println!("{guess}: too high"),
                    Ordering::Equal => {
                        println!("{guess}: correct in {}", guesses_text(guesses));
                        return;
                    }
                }
            }
            _ => println!("{text}: not a number"),
        }
    }
    println!(
        "gave up after {}; the number was {secret}",
        guesses_text(guesses)
    );
}
