const MAX_PLAYERS: u32 = 4; // constants must spell out their type

fn main() {
    let mut count = 0; // `mut` allows it to change
    let birth_year = 1815; // immutable: `birth_year = 1816;` would not compile
    let pi = 3.14159;
    let ready = true;

    count += 3;

    println!("count = {count}");
    println!("birth year = {birth_year}");
    println!("pi = {pi}");
    println!("max players = {MAX_PLAYERS}");
    println!("ready = {ready}");
}
