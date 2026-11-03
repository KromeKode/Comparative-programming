use std::cmp::Reverse;
use std::collections::BTreeSet;

fn yes_no(value: bool) -> &'static str {
    if value { "yes" } else { "no" }
}

fn main() {
    let scores = [
        ("ada", 91),
        ("bob", 78),
        ("cy", 85),
        ("dee", 62),
        ("eve", 99),
        ("finn", 54),
    ];

    // iter() lends the items, filter keeps some, map changes each one, and
    // collect gathers the result. Nothing runs until collect asks.
    let passed: Vec<&str> = scores
        .iter()
        .filter(|&&(_, points)| points >= 70)
        .map(|&(name, _)| name)
        .collect();
    println!("passed: {}", passed.join(", "));

    let total: i32 = scores
        .iter()
        .map(|&(_, points)| points)
        .filter(|&p| p >= 70)
        .sum();
    println!("total of the passes: {total}");

    let &(name, points) = scores.iter().max_by_key(|&&(_, points)| points).unwrap();
    println!("best: {name} with {points}");
    println!(
        "everyone passed: {}",
        yes_no(scores.iter().all(|&(_, p)| p >= 70))
    );
    println!(
        "anyone above 95: {}",
        yes_no(scores.iter().any(|&(_, p)| p > 95))
    );

    let mut ranked = scores.to_vec();
    ranked.sort_by_key(|&(_, points)| Reverse(points));
    let top: Vec<String> = ranked
        .iter()
        .take(3)
        .enumerate()
        .map(|(i, (name, points))| format!("{}. {name} {points}", i + 1))
        .collect();
    println!("top three: {}", top.join(", "));

    // flat_map turns each name into its letters, and a BTreeSet keeps one of
    // each, in order.
    let letters: BTreeSet<char> = scores.iter().flat_map(|(name, _)| name.chars()).collect();
    let letters: Vec<String> = letters.iter().map(|c| c.to_string()).collect();
    println!("letters used: {}", letters.join(" "));
}
