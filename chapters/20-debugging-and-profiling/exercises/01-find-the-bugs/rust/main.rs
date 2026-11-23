// Prints a report of each student's scores. ../README.md says what it should print.
use std::collections::HashMap;

/// Gives total / count, rounded to the nearest whole number, with halves up.
fn rounded_average(total: i64, count: i64) -> i64 {
    (2 * total + count) / (2 * count)
}

fn main() {
    let mut names: Vec<String> = Vec::new(); // in the order they first appear
    let mut scores: HashMap<String, Vec<i64>> = HashMap::new();
    // The input is the file named on the command line, if there is one, so that a
    // debugger can keep the terminal to itself.
    let input = match std::env::args().nth(1) {
        Some(path) => std::fs::read_to_string(path).unwrap(),
        None => std::io::read_to_string(std::io::stdin()).unwrap(),
    };
    for line in input.lines() {
        let mut fields = line.split_whitespace();
        let Some(name) = fields.next() else { continue };
        let marks: Vec<i64> = fields.map(|field| field.parse().unwrap()).collect();
        let all = scores.entry(name.to_string()).or_insert_with(|| {
            names.push(name.to_string());
            Vec::new()
        });
        *all = marks;
    }

    let mut best_name = "-";
    let mut best_average = -1;
    let (mut class_total, mut class_count) = (0, 0);
    let mut total = 0;
    for name in &names {
        let marks = &scores[name];
        for mark in marks {
            total += mark;
        }
        class_total += total;
        class_count += marks.len() as i64;
        let average = rounded_average(total, marks.len() as i64);
        println!("{name} {average}");
        if average >= best_average {
            best_name = name;
            best_average = average;
        }
    }

    if class_count == 0 {
        println!("class average: -");
    } else {
        println!(
            "class average: {}",
            rounded_average(class_total, class_count)
        );
    }
    println!("top: {best_name}");
}
