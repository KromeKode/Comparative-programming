use std::fmt::Write; // lets write! append to a String

fn main() {
    let fruits = ["apple", "banana", "cherry"];
    println!("joined: {}", fruits.join(", "));

    // A String is mutable and grows as needed.
    let mut built = String::new();
    for n in 1..=5 {
        if n > 1 {
            built.push('-');
        }
        write!(built, "{n}").unwrap(); // writing to a String cannot fail
    }
    println!("built: {built}");

    println!("padded: [{:<8}] [{:>8}]", "Ada", 90);
    println!("zero-padded: {:05}", 42);
    println!("two decimals: {:.2}", 3.14159);
}
