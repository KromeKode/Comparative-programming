use std::io;

fn main() {
    let mut line = String::new();
    io::stdin().read_line(&mut line).expect("failed to read stdin");
    let n: u32 = line.trim().parse().expect("expected a whole number");

    for i in 1..=n {
        // A preview of chapter 4: match on both remainders at once.
        match (i % 3, i % 5) {
            (0, 0) => println!("FizzBuzz"),
            (0, _) => println!("Fizz"),
            (_, 0) => println!("Buzz"),
            _ => println!("{i}"),
        }
    }
}
