use std::collections::HashMap;

// A HashMap's order changes from run to run, so print the keys sorted. A BTreeMap
// keeps them sorted all the time.
fn print(label: &str, stock: &HashMap<&str, i32>) {
    let mut keys: Vec<_> = stock.keys().collect();
    keys.sort();
    print!("{label}:");
    for key in keys {
        print!(" {key}={}", stock[key]);
    }
    println!();
}

fn main() {
    let mut stock = HashMap::from([("apple", 3), ("banana", 5)]);
    print("stock", &stock);

    stock.insert("cherry", 7);
    print("add cherry", &stock);

    println!("banana: {}", stock["banana"]); // indexing panics if the key is missing
    match stock.get("durian") {
        Some(count) => println!("durian: {count}"),
        None => println!("durian: 0 (missing)"),
    }

    *stock.entry("apple").or_insert(0) += 2;
    print("apple += 2", &stock);
    stock.remove("banana");
    print("remove banana", &stock);

    let has = stock.contains_key("cherry");
    println!("has cherry: {}", if has { "yes" } else { "no" });
    println!("size: {}", stock.len());
}
