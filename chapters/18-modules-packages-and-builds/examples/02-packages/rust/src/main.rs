use std::io;

fn main() {
    let text = io::read_to_string(io::stdin()).expect("stdin should be text");
    // A dependency's crate is in scope by its name, with no `mod` or `use`.
    println!("textstats {}", textstats::VERSION);
    println!("words: {}", textstats::word_count(&text));
    println!("longest: {}", textstats::longest_word(&text));
}
