fn big_ratio(x: i32, calls: &mut u32) -> bool {
    *calls += 1;
    10 / x > 1 // dividing by zero here would panic
}

fn display_name(name: &str) -> &str {
    // if is an expression, so it does the job of C's ?: operator.
    if name.is_empty() { "stranger" } else { name }
}

fn main() {
    let mut calls = 0; // counts how many times the right side ran
    for x in [0, 5] {
        // && stops as soon as the left side is false, so big_ratio never sees 0.
        let ok = x != 0 && big_ratio(x, &mut calls);
        println!("x = {x}: {}", if ok { "yes" } else { "no" });
    }
    println!("right side ran {calls} of 2 times");
    println!("name \"\": {}", display_name(""));
    println!("name \"Ada\": {}", display_name("Ada"));
}
