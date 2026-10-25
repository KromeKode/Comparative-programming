use std::io;

fn main() {
    let mut line = String::new();
    io::stdin().read_line(&mut line).expect("failed to read stdin");
    let line = line.trim_end_matches(['\r', '\n']);  // read_line keeps the newline
    println!("You typed: {line}");
}
