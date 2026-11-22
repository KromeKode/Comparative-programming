// The compiler finds this module's code in textstats.rs, next to this file.
mod textstats;

use std::io;

fn main() {
    let text = io::read_to_string(io::stdin()).expect("stdin should be text");
    println!("textstats {}", textstats::VERSION);
    println!("words: {}", textstats::word_count(&text));
    println!("longest: {}", textstats::longest_word(&text));
}
