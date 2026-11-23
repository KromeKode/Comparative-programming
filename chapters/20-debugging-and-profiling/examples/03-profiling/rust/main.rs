// Counts the words in its input, and prints the ten most common. It is slow on a long
// text, and section 20.7 runs it under a profiler to find out why.
use std::io::Read;

/// Gives the words of the text: runs of the letters a to z, in lowercase.
fn split_words(text: &str) -> Vec<String> {
    text.to_ascii_lowercase()
        .split(|c: char| !c.is_ascii_lowercase())
        .filter(|word| !word.is_empty())
        .map(String::from)
        .collect()
}

/// Gives the index of the word's count, or `None` if it has none.
fn find(counts: &[(String, usize)], word: &str) -> Option<usize> {
    counts.iter().position(|(w, _)| w == word)
}

fn count_words(words: Vec<String>) -> Vec<(String, usize)> {
    let mut counts: Vec<(String, usize)> = Vec::new();
    for word in words {
        match find(&counts, &word) {
            Some(i) => counts[i].1 += 1,
            None => counts.push((word, 1)),
        }
    }
    counts
}

fn main() {
    let mut text = String::new();
    std::io::stdin().read_to_string(&mut text).unwrap();
    let mut counts = count_words(split_words(&text));
    counts.sort_by(|a, b| b.1.cmp(&a.1).then_with(|| a.0.cmp(&b.0)));
    for (word, n) in counts.iter().take(10) {
        println!("{n} {word}");
    }
}
