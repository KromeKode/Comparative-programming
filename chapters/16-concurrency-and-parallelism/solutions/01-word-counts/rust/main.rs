use std::collections::HashMap;
use std::io::{self, Read};
use std::thread;

fn plural(count: usize, noun: &str) -> String {
    if count == 1 {
        format!("{count} {noun}")
    } else {
        format!("{count} {noun}s")
    }
}

// Runs on a thread of its own. The lines are borrowed, and the map is its own.
fn count_words(lines: &[&str]) -> HashMap<String, usize> {
    let mut counts = HashMap::new();
    for line in lines {
        for word in line
            .split(|c: char| !c.is_ascii_alphabetic())
            .filter(|w| !w.is_empty())
        {
            *counts.entry(word.to_ascii_lowercase()).or_insert(0) += 1;
        }
    }
    counts
}

fn parse_first(line: Option<&&str>) -> Option<(usize, usize)> {
    let words: Vec<&str> = line?.split_whitespace().collect();
    let is_number = |word: &str| word.bytes().all(|b| b.is_ascii_digit());
    match words[..] {
        [workers, top] if is_number(workers) && is_number(top) => {
            Some((workers.parse().ok()?, top.parse().ok()?))
        }
        _ => None,
    }
}

fn main() {
    let mut input = String::new();
    io::stdin()
        .read_to_string(&mut input)
        .expect("can't read stdin");
    let lines: Vec<&str> = input.lines().collect();
    let Some((workers, top)) = parse_first(lines.first()) else {
        println!("error: the first line must be two numbers, workers and top");
        return;
    };
    if !(1..=16).contains(&workers) {
        println!("error: workers must be from 1 to 16");
        return;
    }

    // Each worker gets text.len() / workers lines, and the first few get one
    // more, so that every line is counted once.
    let text = lines.get(1..).unwrap_or_default();
    let mut chunks = Vec::new();
    let mut rest = text;
    for i in 0..workers {
        let size = text.len() / workers + usize::from(i < text.len() % workers);
        let (chunk, after) = rest.split_at(size);
        chunks.push(chunk);
        rest = after;
    }

    // A scoped thread can borrow from main, because the scope waits for every
    // thread in it before it returns, so the lines outlive the threads.
    let counts: Vec<HashMap<String, usize>> = thread::scope(|scope| {
        let handles: Vec<_> = chunks
            .iter()
            .map(|chunk| scope.spawn(|| count_words(chunk)))
            .collect();
        handles
            .into_iter()
            .map(|handle| handle.join().unwrap())
            .collect()
    });

    println!(
        "{}, {}",
        plural(workers, "worker"),
        plural(text.len(), "line")
    );
    let mut total: HashMap<String, usize> = HashMap::new();
    for (i, (chunk, count)) in chunks.iter().zip(counts).enumerate() {
        let words = count.values().sum();
        println!(
            "worker {i}: {}, {}",
            plural(chunk.len(), "line"),
            plural(words, "word")
        );
        for (word, n) in count {
            *total.entry(word).or_insert(0) += n;
        }
    }
    println!(
        "{}, {} different",
        plural(total.values().sum(), "word"),
        total.len()
    );

    let mut sorted: Vec<(String, usize)> = total.into_iter().collect();
    sorted.sort_by(|a, b| b.1.cmp(&a.1).then_with(|| a.0.cmp(&b.0)));
    for (word, count) in sorted.iter().take(top) {
        println!("{word}: {count}");
    }
}
