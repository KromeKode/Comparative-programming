use std::collections::HashMap;
use std::io::{self, BufRead};

// The closure owns `next` (because of move) and changes it, so it is FnMut.
fn make_counter(start: i64, step: i64) -> impl FnMut() -> i64 {
    let mut next = start;
    move || {
        let value = next;
        next += step;
        value
    }
}

fn main() {
    // Every closure has its own unnamed type. To keep many in one map,
    // put each in a Box and store them as `dyn FnMut`.
    let mut counters: HashMap<String, Box<dyn FnMut() -> i64>> = HashMap::new();
    for line in io::stdin().lock().lines() {
        let line = line.expect("failed to read stdin");
        let words: Vec<&str> = line.split_whitespace().collect();
        match words[..] {
            ["new", name, start, step] => {
                let start = start.parse().expect("start must be a whole number");
                let step = step.parse().expect("step must be a whole number");
                counters.insert(name.to_string(), Box::new(make_counter(start, step)));
            }
            ["next", name] => match counters.get_mut(name) {
                Some(counter) => println!("{name}: {}", counter()),
                None => println!("{name}: no such counter"),
            },
            [] => {} // a blank line
            _ => panic!("unknown command: {line}"),
        }
    }
}
