use std::mem;

fn add_each(values: &mut [i32], amount: i32) {
    for item in values.iter_mut() {
        *item += amount;
    }
}

fn main() {
    let mut x = 1;
    let a = &x;
    let b = &x; // many shared references may alias, because none can write
    println!("two shared references: *a = {a}, *b = {b}");

    // A &mut must be the only way to reach x while it is in use.
    let only = &mut x;
    *only = 5;
    println!("after *only = 5: x = {x}");

    let mut y = 3;
    mem::swap(&mut x, &mut y);
    println!("after mem::swap(&mut x, &mut y): x = {x}, y = {y}");

    // mem::swap(&mut values[0], &mut values[2]) is refused: two &mut into one
    // Vec at once. The Vec's own swap method takes indexes instead.
    let mut values = vec![1, 2, 3];
    values.swap(0, 2);
    println!("after values.swap(0, 2): {values:?}");

    // add_each(&mut values, &values[0]) is refused too, so the amount is copied.
    let amount = values[0];
    add_each(&mut values, amount);
    println!("add_each(&mut values, values[0]): {values:?}");

    // split_at_mut proves to the compiler that two &mut do not overlap.
    let (left, right) = values.split_at_mut(1);
    mem::swap(&mut left[0], &mut right[1]);
    println!("after swapping across split_at_mut(1): {values:?}");
}
