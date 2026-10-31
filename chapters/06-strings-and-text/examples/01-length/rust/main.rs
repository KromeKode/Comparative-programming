fn main() {
    // A Rust string is always valid UTF-8. \u{...} names a code point.
    let s = "Zo\u{eb} \u{1F44D}\u{1F3FD}";

    println!("text: {s}");
    println!("built-in length: {} (bytes)", s.len());
    println!("UTF-8 bytes: {}", s.len());
    println!("UTF-16 code units: {}", s.encode_utf16().count());
    println!("code points: {}", s.chars().count()); // a char is one code point
}
