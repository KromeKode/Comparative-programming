use std::collections::HashMap;
use std::io::{self, Read};

fn main() {
    let mut text = String::new();
    io::stdin()
        .read_to_string(&mut text)
        .expect("failed to read stdin");

    // A word is a run of ASCII letters and non-ASCII characters.
    let is_word_char = |c: char| c.is_ascii_alphabetic() || !c.is_ascii();
    let mut counts: HashMap<String, usize> = HashMap::new();
    for word in text.split(|c| !is_word_char(c)).filter(|w| !w.is_empty()) {
        *counts.entry(word.to_ascii_lowercase()).or_insert(0) += 1;
    }

    // Most frequent first; ties in code point order, which is how str compares.
    let mut sorted: Vec<(String, usize)> = counts.into_iter().collect();
    sorted.sort_by(|a, b| b.1.cmp(&a.1).then_with(|| a.0.cmp(&b.0)));
    for (word, count) in sorted {
        println!("{count} {word}");
    }
}
