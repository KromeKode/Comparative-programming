fn add_one(x: i32) -> i32 {
    x + 1
}

fn triple(x: i32) -> i32 {
    x * 3
}

// impl Fn(i32) -> i32 accepts a named function or a closure.
fn apply_twice(f: impl Fn(i32) -> i32, x: i32) -> i32 {
    f(f(x))
}

// Returning a closure: `move` copies outer and inner into it, so it can outlive this call.
fn compose(outer: impl Fn(i32) -> i32, inner: impl Fn(i32) -> i32) -> impl Fn(i32) -> i32 {
    move |x| outer(inner(x))
}

fn main() {
    println!("apply_twice(add_one, 5) = {}", apply_twice(add_one, 5));
    println!("apply_twice(triple, 5) = {}", apply_twice(triple, 5));
    let square = |x: i32| x * x; // a closure: a function with no name
    println!("apply_twice(square, 3) = {}", apply_twice(square, 3));
    let add_then_triple = compose(triple, add_one);
    println!("compose(triple, add_one)(5) = {}", add_then_triple(5));
}
