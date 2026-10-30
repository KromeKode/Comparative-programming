use std::io::{self, BufRead};

// The five closures below all have different types, so return them boxed.
fn make_step(op: &str, n: i64) -> Box<dyn Fn(i64) -> i64> {
    match op {
        "add" => Box::new(move |x| x + n),
        "sub" => Box::new(move |x| x - n),
        "mul" => Box::new(move |x| x * n),
        "min" => Box::new(move |x| x.min(n)),
        "max" => Box::new(move |x| x.max(n)),
        _ => panic!("unknown step: {op}"),
    }
}

fn main() {
    for line in io::stdin().lock().lines() {
        let line = line.expect("failed to read stdin");
        let words: Vec<&str> = line.split_whitespace().collect();
        let Some((first, rest)) = words.split_first() else {
            continue; // a blank line
        };
        let start: i64 = first.parse().expect("the start must be a whole number");
        let mut steps = Vec::new();
        for pair in rest.chunks(2) {
            let [op, n] = pair else {
                panic!("a step needs an operation and a number");
            };
            steps.push(make_step(op, n.parse().expect("not a whole number")));
        }
        let mut value = start;
        for step in &steps {
            value = step(value);
        }
        println!("{start} -> {value}");
    }
}
