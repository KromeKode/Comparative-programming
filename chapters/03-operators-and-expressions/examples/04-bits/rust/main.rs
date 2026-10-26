const READ: u8 = 0b001;
const WRITE: u8 = 0b010;
const EXEC: u8 = 0b100;

fn yes_no(condition: bool) -> &'static str {
    if condition { "yes" } else { "no" }
}

fn main() {
    let mut perms = READ | WRITE; // turn bits on with |
    println!("read | write: {perms:08b}");
    // & binds tighter than == in Rust, so no parentheses are needed.
    println!("has write? {}", yes_no(perms & WRITE != 0));
    println!("has exec? {}", yes_no(perms & EXEC != 0));
    perms &= !WRITE; // Rust writes bitwise NOT as !, not ~
    println!("clear write: {perms:08b}");
    perms ^= EXEC; // flip a bit with ^
    println!("toggle exec: {perms:08b}");
    println!("5 << 2 = {}", 5 << 2);
    println!("-16 >> 2 = {}", -16 >> 2); // signed types copy the sign bit in
    println!("set bits in 10110: {}", 0b10110_u32.count_ones());
}
