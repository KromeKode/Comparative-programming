fn main() {
    let parsed: i32 = "42".parse().expect("not a number"); // parse returns a Result
    println!("\"42\" + 1 as a number: {}", parsed + 1);

    let x: f64 = 3.99;
    println!("3.99 truncated: {}", x as i32); // `as` drops the fraction
    println!("-3.99 truncated: {}", -x as i32);
    println!("2.5 rounded: {}", 2.5_f64.round() as i64); // halfway rounds away from zero
    println!("-2.5 rounded: {}", (-2.5_f64).round() as i64);

    let seven: i32 = 7;
    let two: i32 = 2;
    println!("7 / 2 as integers: {}", seven / two);
    println!("7 / 2 as floats: {}", seven as f64 / two as f64); // no automatic conversion
}
