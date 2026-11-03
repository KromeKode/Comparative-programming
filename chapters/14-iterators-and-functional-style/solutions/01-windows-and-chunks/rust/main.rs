use std::collections::VecDeque;
use std::io::{self, BufRead};
use std::iter::Peekable;

// Each adapter wraps any iterator of numbers and pulls one item at a time,
// keeping only what it needs.
struct Windows<I> {
    inner: I,
    k: usize,
    window: VecDeque<i64>,
}

impl<I: Iterator<Item = i64>> Iterator for Windows<I> {
    type Item = Vec<i64>;

    fn next(&mut self) -> Option<Vec<i64>> {
        if self.window.len() == self.k {
            self.window.pop_front(); // the oldest item leaves before the next one comes in
        }
        while self.window.len() < self.k {
            self.window.push_back(self.inner.next()?);
        }
        Some(self.window.iter().copied().collect())
    }
}

struct Chunks<I> {
    inner: I,
    k: usize,
}

impl<I: Iterator<Item = i64>> Iterator for Chunks<I> {
    type Item = Vec<i64>;

    fn next(&mut self) -> Option<Vec<i64>> {
        let chunk: Vec<i64> = self.inner.by_ref().take(self.k).collect();
        (!chunk.is_empty()).then_some(chunk)
    }
}

// Peekable lets runs look at the next item without taking it.
struct Runs<I: Iterator> {
    inner: Peekable<I>,
}

impl<I: Iterator<Item = i64>> Iterator for Runs<I> {
    type Item = Vec<i64>;

    fn next(&mut self) -> Option<Vec<i64>> {
        let first = self.inner.next()?;
        let mut run = vec![first];
        while let Some(item) = self.inner.next_if_eq(&first) {
            run.push(item);
        }
        Some(run)
    }
}

// An extension trait adds the adapters to every iterator of numbers, so they
// chain like the standard ones: numbers.windows(3).map(...).
trait Groups: Iterator<Item = i64> + Sized {
    fn windows(self, k: usize) -> Windows<Self> {
        Windows {
            inner: self,
            k,
            window: VecDeque::with_capacity(k),
        }
    }

    fn chunks(self, k: usize) -> Chunks<Self> {
        Chunks { inner: self, k }
    }

    fn runs(self) -> Runs<Self> {
        Runs {
            inner: self.peekable(),
        }
    }
}

impl<I: Iterator<Item = i64>> Groups for I {}

fn show(groups: impl Iterator<Item = Vec<i64>>) -> String {
    let texts: Vec<String> = groups
        .map(|group| {
            let items: Vec<String> = group.iter().map(|item| item.to_string()).collect();
            format!("[{}]", items.join(" "))
        })
        .collect();
    texts.join(" ")
}

fn answer(words: &[&str]) -> String {
    let number = |word: &&str| word.parse::<i64>().expect("a whole number");
    let text = if words[0] == "runs" {
        show(words[1..].iter().map(number).runs())
    } else {
        let k: usize = words[1].parse().expect("a size");
        let numbers = words[2..].iter().map(number);
        match words[0] {
            "windows" => show(numbers.windows(k)),
            "chunks" => show(numbers.chunks(k)),
            _ => {
                let sums: Vec<String> = numbers
                    .windows(k)
                    .map(|w| w.iter().sum::<i64>().to_string())
                    .collect();
                sums.join(" ")
            }
        }
    };
    if text.is_empty() {
        "(none)".to_string()
    } else {
        text
    }
}

fn main() {
    for line in io::stdin().lock().lines() {
        let line = line.expect("readable input");
        let words: Vec<&str> = line.split_whitespace().collect();
        if !words.is_empty() {
            println!("{}", answer(&words));
        }
    }
}
