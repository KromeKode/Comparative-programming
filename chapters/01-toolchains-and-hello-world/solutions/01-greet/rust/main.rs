use std::io;

fn main() {
    let mut line = String::new();
    io::stdin().read_line(&mut line).expect("failed to read stdin");

    let name = line.trim();
    if name.is_empty() {
        println!("Hello, stranger!");
    } else {
        println!("Hello, {name}!");
    }
}
