fn yes_no(condition: bool) -> &'static str {
    if condition { "yes" } else { "no" }
}

fn main() {
    let a = String::from("hello");
    let mut b = String::from("hel");
    b.push_str("lo"); // built separately, same characters
    // == compares contents for every type that implements PartialEq.
    println!("strings with ==: {}", yes_no(a == b));
    // std::ptr::eq asks a different question: are these the same place in memory?
    let same_place = std::ptr::eq(&a, &b);
    println!("strings with std::ptr::eq: {}", yes_no(same_place));

    let xs = vec![1, 2, 3];
    let ys = vec![1, 2, 3];
    println!("vectors with ==: {}", yes_no(xs == ys));

    let nan = f64::NAN;
    println!("NaN == NaN: {}", yes_no(nan == nan));
}
