// Prints when it is dropped, to show that a moved value is dropped once.
struct Token(&'static str);

impl Drop for Token {
    fn drop(&mut self) {
        println!("  drop {}", self.0);
    }
}

// Takes ownership of v. When the function ends, v is dropped.
fn keep(v: Vec<i32>) -> usize {
    v.len()
}

// Takes ownership, changes the vector, and gives it back.
fn with_six(mut v: Vec<i32>) -> Vec<i32> {
    v.push(6);
    v
}

fn main() {
    let a = vec![1, 2, 3];
    let b = a.clone(); // a copy, and you have to ask for it
    let c = a; // a move: c owns the heap block, and a cannot be used again
    println!("clone then move: b = {b:?}, c = {c:?}");

    let p = Box::new(7);
    let q = p;
    println!("after the move, q = {q}, and p is gone");

    let big = vec![1, 2, 3, 4, 5];
    println!("keep(big.clone()) = {}", keep(big.clone()));
    println!("keep(big) = {}, and big is gone", keep(big));

    let v = with_six(vec![4, 5]);
    println!("with_six gave back {v:?}");

    // To move out of something that must stay usable, leave a value behind.
    let mut slot = Some(String::from("job"));
    let job = slot.take();
    let mut list = vec![1, 2];
    let old = std::mem::take(&mut list);
    println!("take: slot = {slot:?}, job = {job:?}, list = {list:?}, old = {old:?}");

    println!("a value is dropped by its last owner, once:");
    {
        let first = Token("token");
        let second = first;
        println!("  moved from first to second");
        drop(second);
        println!("  first's scope ends, and nothing is dropped twice");
    }

    // Box::into_raw gives up ownership and returns a raw pointer. Nothing owns
    // the value until Box::from_raw takes it back, and forgetting to would
    // leak it.
    let boxed = Box::new(String::from("on the heap"));
    let raw: *mut String = Box::into_raw(boxed);
    let back = unsafe { Box::from_raw(raw) };
    println!("into_raw, then from_raw: {back}");
}
