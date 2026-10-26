fn main() {
    let i32_max = i32::MAX;
    println!("int32 max: {i32_max}");

    // Plain `i32_max + 1` panics in a debug build and wraps in a release build.
    // Say which behavior you want instead:
    println!("checked_add(1): {:?}", i32_max.checked_add(1));
    println!("wrapping_add(1): {}", i32_max.wrapping_add(1));
    println!("saturating_add(1): {}", i32_max.saturating_add(1));

    println!("int64 max: {}", i64::MAX);
}
