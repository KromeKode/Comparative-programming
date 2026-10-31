fn main() {
    let mut numbers = vec![3, 1, 4, 1, 5];
    println!("numbers: {numbers:?}"); // {:?} prints a Vec as [3, 1, 4, 1, 5]
    println!("length: {}", numbers.len());
    // Indexing panics on an empty Vec; first() and last() return an Option instead.
    let (first, last) = (numbers[0], numbers[numbers.len() - 1]);
    println!("first: {first}, last: {last}");

    numbers.push(9);
    println!("append 9: {numbers:?}");
    numbers.insert(1, 2);
    println!("insert 2 at index 1: {numbers:?}");
    let removed = numbers.remove(0);
    println!("remove index 0 (was {removed}): {numbers:?}");

    let has_four = if numbers.contains(&4) { "yes" } else { "no" };
    println!("contains 4: {has_four}");
    let first_one = numbers.iter().position(|&n| n == 1).unwrap();
    println!("index of 1: {first_one}");

    numbers.sort();
    println!("sorted: {numbers:?}");
    println!("slice 1..4: {:?}", &numbers[1..4]); // a borrowed slice, no copy

    numbers.reverse();
    println!("reversed: {numbers:?}");
}
