// `move` makes the closure own its captured variables, so it can be returned.
fn make_adder(n: i32) -> impl Fn(i32) -> i32 {
    move |x| x + n
}

// A closure that changes what it captured is FnMut, and must be called through `mut`.
fn make_accumulator() -> impl FnMut(i32) -> i32 {
    let mut total = 0;
    move |x| {
        total += x;
        total
    }
}

fn main() {
    let add5 = make_adder(5);
    let add10 = make_adder(10);
    println!("add5(10) = {}", add5(10));
    println!("add10(10) = {}", add10(10));

    let mut running = make_accumulator();
    let (a, b, c) = (running(3), running(5), running(2));
    println!("running total: {a} {b} {c}");

    // Each closure has its own unnamed type, so a Vec of them stores Box<dyn Fn>.
    let mut made: Vec<Box<dyn Fn() -> i32>> = Vec::new();
    for i in 0..3 {
        made.push(Box::new(move || i));
    }
    let values: Vec<String> = made.iter().map(|f| f().to_string()).collect();
    println!("made in a loop: {}", values.join(" "));
}
