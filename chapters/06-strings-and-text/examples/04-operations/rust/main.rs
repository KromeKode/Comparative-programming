fn yes_no(condition: bool) -> &'static str {
    if condition { "yes" } else { "no" }
}

fn main() {
    let text = "  The quick  brown fox  ";

    let trimmed = text.trim(); // a &str pointing into text: nothing is copied
    println!("trimmed: [{trimmed}]");

    let words: Vec<&str> = trimmed.split_whitespace().collect();
    println!("words: {}", words.join("|"));

    println!("upper: {}", trimmed.to_uppercase());
    println!("contains \"quick\": {}", yes_no(trimmed.contains("quick")));
    // find returns Option<usize>: a byte index, or None.
    println!("index of \"brown\": {}", trimmed.find("brown").unwrap());
    println!("replaced: {}", trimmed.replace("quick", "slow"));
    println!(
        "starts with \"The\": {}",
        yes_no(trimmed.starts_with("The"))
    );

    let mut fruits = vec!["banana", "apple", "Cherry"];
    fruits.sort(); // compares bytes
    println!("sorted: {}", fruits.join(" "));
    fruits.sort_by_key(|s| s.to_lowercase());
    println!("sorted ignoring case: {}", fruits.join(" "));
}
