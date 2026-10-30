// Return several values as a tuple.
fn min_max(values: &[i32]) -> (i32, i32) {
    let mut lowest = values[0];
    let mut highest = values[0];
    for &value in values {
        lowest = lowest.min(value);
        highest = highest.max(value);
    }
    (lowest, highest)
}

fn div_mod(a: i32, b: i32) -> (i32, i32) {
    (a / b, a % b)
}

fn main() {
    let (lowest, highest) = min_max(&[3, -1, 4, 1, 5]); // a pattern unpacks the tuple
    println!("min and max of [3, -1, 4, 1, 5]: {lowest} {highest}");

    let (quotient, remainder) = div_mod(17, 5);
    println!("17 divided by 5: {quotient} remainder {remainder}");

    let mut first = "hello";
    let mut second = "world";
    (first, second) = (second, first); // destructuring assignment swaps them
    println!("swapped: {first} {second}");
}
