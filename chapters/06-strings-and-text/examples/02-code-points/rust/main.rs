fn main() {
    let s = "Zo\u{eb} \u{1F44D}\u{1F3FD}";

    let bytes: Vec<String> = s.bytes().map(|b| format!("{b:02X}")).collect();
    println!("UTF-8 bytes: {}", bytes.join(" "));

    let code_points: Vec<String> = s.chars().map(|c| format!("U+{:04X}", c as u32)).collect();
    println!("code points: {}", code_points.join(" "));

    // &s[..3] would panic: byte 3 is in the middle of 'ë'. Count chars instead.
    let first_three: String = s.chars().take(3).collect();
    println!("first 3 code points: {first_three}");
}
