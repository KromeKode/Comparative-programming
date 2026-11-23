// Counts the letters in its input, and draws a bar for each one. It expects lowercase
// text, and doesn't check: section 20.3 gives it capitals, to see what happens.
use std::io::Read;

fn count_letters(text: &str) -> [usize; 26] {
    let mut counts = [0; 26];
    for c in text.chars() {
        if c.is_alphabetic() {
            counts[c as usize - 'a' as usize] += 1;
        }
    }
    counts
}

fn main() {
    let mut text = String::new();
    std::io::stdin().read_to_string(&mut text).unwrap();
    let counts = count_letters(&text);
    for (i, &n) in counts.iter().enumerate() {
        if n > 0 {
            println!("{} {} {}", (b'a' + i as u8) as char, n, "#".repeat(n));
        }
    }
}
