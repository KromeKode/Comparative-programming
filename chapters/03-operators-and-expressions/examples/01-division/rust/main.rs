fn main() {
    // Integer division truncates toward zero, as in C.
    println!("7 / 2 = {}", 7 / 2);
    println!("-7 / 2 = {}", -7 / 2);
    // The remainder takes the sign of the left operand.
    println!("7 % 3 = {}", 7 % 3);
    println!("-7 % 3 = {}", -7 % 3);
    println!("7 % -3 = {}", 7 % -3);
    // Rust never converts between number types for you: 7.0 / 2 does not compile.
    println!("7.0 / 2.0 = {}", 7.0 / 2.0);
    // Euclidean division: the remainder is never negative.
    println!("(-7).div_euclid(2) = {}", (-7_i32).div_euclid(2));
    println!("(-7).rem_euclid(3) = {}", (-7_i32).rem_euclid(3));
}
