//! The textstats library crate. Everything in it is private to the crate unless
//! marked `pub`, and the crates that depend on it see only what is.

pub const VERSION: &str = "1.0";

/// The words in text: runs of ASCII letters. Private, so only this module can
/// call it, and it can change without breaking anyone.
fn words(text: &str) -> impl Iterator<Item = &str> {
    text.split(|c: char| !c.is_ascii_alphabetic())
        .filter(|word| !word.is_empty())
}

/// The number of words in text.
pub fn word_count(text: &str) -> usize {
    words(text).count()
}

/// The first of the longest words in text.
pub fn longest_word(text: &str) -> &str {
    words(text).fold(
        "",
        |best, word| if word.len() > best.len() { word } else { best },
    )
}
