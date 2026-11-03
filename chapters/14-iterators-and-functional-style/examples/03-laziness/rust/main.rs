// Both functions say when they run, so the output shows the order of the work.
fn square(n: i32) -> i32 {
    println!("  square {n}");
    n * n
}

fn even(x: &i32) -> bool {
    println!("  even? {x}");
    x % 2 == 0
}

fn main() {
    // Eager: collect finishes each step before the next one starts.
    println!("eager:");
    let squares: Vec<i32> = (1..=5).map(square).collect();
    let evens: Vec<i32> = squares.into_iter().filter(even).collect();
    println!("  first two: {} {}", evens[0], evens[1]);

    // Lazy: map and filter only describe the work. collect pulls items through
    // one at a time, and take stops pulling after two.
    println!("lazy:");
    let first_two: Vec<i32> = (1..=5).map(square).filter(even).take(2).collect();
    println!("  first two: {} {}", first_two[0], first_two[1]);

    // 1.. counts up forever, so only a lazy pipeline can use it.
    let endless: Vec<String> = (1..)
        .map(|n| n * n)
        .filter(|x| x % 2 == 0 && *x > 50)
        .take(3)
        .map(|x: i32| x.to_string())
        .collect();
    println!("endless: {}", endless.join(" "));
}
