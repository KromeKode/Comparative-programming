fn main() {
    let mut x = 5;
    let r = &mut x; // a mutable reference: the only way to change x while r is in use
    *r = 7;
    println!("after *r = 7: x = {x}");

    // Shared references can only read, and any number of them may exist at once.
    let a = &x;
    let b = &x;
    println!("*a + *b = {}", *a + *b);

    // A slice is a pointer plus a length, so it knows where it ends.
    let scores = [90, 85, 77, 68];
    let middle = &scores[1..3];
    println!("middle = {middle:?}, middle[1] = {}", middle[1]);

    println!(
        "size_of_val: x {}, a {}, scores {}, middle {}",
        size_of_val(&x),
        size_of_val(&a),
        size_of_val(&scores),
        size_of_val(&middle)
    );

    // Raw pointers exist too. Making one is safe; reading through one is not.
    let p = &raw const x;
    let value = unsafe { *p };
    println!("through a raw pointer: {value}");

    // References are never null. "Maybe no reference" is an Option (chapter 8).
    let nowhere: Option<&i32> = None;
    println!("nowhere is {nowhere:?}");
}
