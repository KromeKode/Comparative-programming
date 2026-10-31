use std::io::{self, BufRead};

// chars() walks code points, so a multi-byte character stays in one piece.
fn is_palindrome(text: &str) -> bool {
    let kept: Vec<char> = text
        .chars()
        .filter(|c| !c.is_ascii() || c.is_ascii_alphanumeric())
        .map(|c| c.to_ascii_lowercase()) // leaves non-ASCII characters alone
        .collect();
    kept.iter().eq(kept.iter().rev())
}

fn main() {
    for line in io::stdin().lock().lines() {
        let line = line.expect("failed to read stdin");
        let text = line.trim();
        if text.is_empty() {
            continue;
        }
        let verdict = if is_palindrome(text) {
            "palindrome"
        } else {
            "not a palindrome"
        };
        println!("{text}: {verdict}");
    }
}
