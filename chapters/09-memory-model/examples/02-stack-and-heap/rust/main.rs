// Static data: one copy for the whole run.
static GREETING: &str = "hello";

// Each call gets its own stack frame with its own `here`.
fn countdown(n: i32) {
    let here = n * 10;
    if n > 0 {
        countdown(n - 1);
    }
    print!(" {here}");
}

fn main() {
    print!("each frame keeps its own local:");
    countdown(3);
    println!();

    let on_stack = [0i32; 1000]; // 4000 bytes inside main's frame
    let on_heap = vec![0i32; 1000]; // a small header here, the items on the heap
    println!("[i32; 1000]: {} bytes", size_of_val(&on_stack));
    println!(
        "Vec<i32> of 1000: {} bytes here, {} on the heap",
        size_of_val(&on_heap),
        on_heap.len() * size_of::<i32>()
    );

    // The header holds a pointer, the capacity, and the length. Pushing past the
    // capacity moves the items to a bigger block on the heap.
    let mut growing = Vec::new();
    let mut capacity = growing.capacity();
    print!("capacity as 100 items are added:");
    for i in 0..100 {
        growing.push(i);
        if growing.capacity() != capacity {
            capacity = growing.capacity();
            print!(" {capacity}");
        }
    }
    println!();
    println!("size_of_val(&growing) is still {}", size_of_val(&growing));

    // Box puts one value on the heap and frees it when `boxed` goes away (chapter 10).
    let boxed = Box::new(42);
    let size = size_of_val(&boxed);
    println!("*boxed = {}, size_of_val(&boxed) = {size}", *boxed);

    // A String owns heap memory; a &str can point into static data instead.
    let owned = String::from(GREETING);
    println!(
        "String: {} bytes, &str: {} bytes, both say {owned}",
        size_of_val(&owned),
        size_of_val(&GREETING)
    );
}
