fn bump_copy(mut n: i32) {
    n += 1;
    println!("inside bump_copy: n = {n}");
}

// A mutable reference lets the function change the caller's variable.
fn bump(n: &mut i32) {
    *n += 1;
}

// Passing a Vec by value hands it over (chapter 10), so the caller clones it
// first if it wants to keep its own.
fn add_copy(mut v: Vec<i32>) {
    v.push(4);
    println!("inside add_copy: {} items", v.len());
}

fn add(v: &mut Vec<i32>) {
    v.push(4);
}

// A shared reference to a slice reads the items without copying them.
fn total(v: &[i32]) -> i32 {
    v.iter().sum()
}

fn main() {
    let mut n = 1;
    bump_copy(n);
    println!("after bump_copy(n): n = {n}");
    bump(&mut n); // the call says that n may change
    println!("after bump(&mut n): n = {n}");

    let mut v = vec![1, 2, 3];
    add_copy(v.clone());
    println!("after add_copy(v.clone()): v has {} items", v.len());
    add(&mut v);
    println!("after add(&mut v): v has {} items", v.len());
    println!("total(&v) = {}", total(&v));
}
