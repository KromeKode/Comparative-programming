// The lifetime 'a says: the result borrows from a or b, so it can be used
// only while both are alive. The compiler checks every caller.
fn longest<'a>(a: &'a str, b: &'a str) -> &'a str {
    if b.len() > a.len() { b } else { a }
}

// Walks the words of a string without copying them. Words<'a> borrows the
// text, and the compiler will not let the text go away first.
struct Words<'a> {
    rest: &'a str,
}

impl<'a> Iterator for Words<'a> {
    type Item = &'a str;

    fn next(&mut self) -> Option<&'a str> {
        let rest = self.rest.trim_start_matches(' ');
        if rest.is_empty() {
            return None;
        }
        let end = rest.find(' ').unwrap_or(rest.len());
        let (word, after) = rest.split_at(end);
        self.rest = after;
        Some(word)
    }
}

// One reference goes in and one comes out, so the compiler can tell they share
// a lifetime. Words<'_> shows that the result borrows. Plain Words would get a
// mismatched_lifetime_syntaxes warning.
fn words(text: &str) -> Words<'_> {
    Words { rest: text }
}

fn main() {
    let a = String::from("apple");
    let b = String::from("fig");
    let result = longest(&a, &b);
    println!("longest(\"{a}\", \"{b}\") = {result}");

    let text = String::from("the quick brown fox");
    print!("words of \"{text}\":");
    for word in words(&text) {
        print!(" [{word}]");
    }
    println!();

    // The words are slices of text, not copies of it.
    if let Some(second) = words(&text).nth(1) {
        let offset = second.as_ptr() as usize - text.as_ptr() as usize;
        println!("\"{second}\" starts {offset} bytes into the text");
    }

    // A borrow lasts until its last use, not to the end of the scope, so the
    // push is allowed: first is not used after it.
    let mut scores = vec![90, 85];
    let first = &scores[0];
    println!("first = {first}");
    scores.push(70);
    println!("after the push, scores = {scores:?}");
}
