use std::collections::HashSet;

// A HashSet has no order, so sort a copy of the items to print them.
fn print(label: &str, items: &HashSet<i32>) {
    let mut sorted: Vec<_> = items.iter().collect();
    sorted.sort();
    print!("{label}:");
    for n in sorted {
        print!(" {n}");
    }
    println!();
}

fn main() {
    let a = HashSet::from([1, 2, 3, 4]);
    let b = HashSet::from([3, 4, 5]);
    print("a", &a);
    print("b", &b);
    print("union", &(&a | &b)); // the operators work on references to two sets
    print("intersection", &(&a & &b));
    print("a minus b", &(&a - &b));
    println!("3 in a: {}", if a.contains(&3) { "yes" } else { "no" });

    let unique: HashSet<i32> = [3, 1, 3, 2, 1].into_iter().collect();
    print("unique of 3 1 3 2 1, sorted", &unique);

    // Tuples can be set members and map keys, because they hash and compare item by item.
    let path = [(0, 0), (1, 0), (0, 0), (1, 1)];
    let visited: HashSet<(i32, i32)> = path.into_iter().collect();
    println!("visited {} points, {} distinct", path.len(), visited.len());
    let seen = visited.contains(&(1, 0));
    println!("(1, 0) visited: {}", if seen { "yes" } else { "no" });
}
