// Rust has no default arguments. Option<T> marks a parameter the caller may skip.
fn greet(name: &str, greeting: Option<&str>, punctuation: Option<&str>) -> String {
    let greeting = greeting.unwrap_or("Hello");
    let punctuation = punctuation.unwrap_or("!");
    format!("{greeting}, {name}{punctuation}")
}

// Rust has no variadic functions either (println! is a macro, not a function).
// A slice parameter accepts an array literal, a Vec, or part of either.
fn total(values: &[i32]) -> i32 {
    let mut sum = 0;
    for value in values {
        sum += value;
    }
    sum
}

fn main() {
    println!("{}", greet("Ada", None, None));
    println!("{}", greet("Grace", Some("Hi"), None));
    println!("{}", greet("Linus", None, Some("?")));

    println!("total() = {}", total(&[]));
    println!("total(1, 2, 3) = {}", total(&[1, 2, 3]));
    let numbers = vec![4, 3, 2, 1];
    println!("total of [4, 3, 2, 1] = {}", total(&numbers));
}
