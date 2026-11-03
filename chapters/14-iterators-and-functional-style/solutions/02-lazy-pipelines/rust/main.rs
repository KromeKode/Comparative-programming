use std::io::{self, BufRead};
use std::iter;

// Every stage changes the iterator's type, so the pipeline is built at run
// time out of boxed trait objects (chapter 13): one type for any iterator.
type Items = Box<dyn Iterator<Item = i64>>;

fn is_prime(n: i64) -> bool {
    n >= 2 && (2..).take_while(|d| d * d <= n).all(|d| n % d != 0)
}

fn number(word: &str) -> i64 {
    word.parse().expect("a whole number")
}

fn source(words: &[&str]) -> Items {
    match words {
        ["naturals"] => Box::new(1..),
        ["primes"] => Box::new((2..).filter(|&n| is_prime(n))),
        ["fibonacci"] => {
            Box::new(iter::successors(Some((0, 1)), |&(a, b)| Some((b, a + b))).map(|(a, _)| a))
        }
        ["range", low, high] => Box::new(number(low)..=number(high)),
        ["repeat", n] => Box::new(iter::repeat(number(n))),
        _ => panic!("unknown source {words:?}"),
    }
}

fn predicate(words: &[&str]) -> Box<dyn Fn(&i64) -> bool> {
    match words {
        ["even"] => Box::new(|x| x % 2 == 0),
        ["odd"] => Box::new(|x| x % 2 != 0),
        ["prime"] => Box::new(|&x| is_prime(x)),
        ["above", n] => {
            let n = number(n);
            Box::new(move |&x| x > n)
        }
        ["below", n] => {
            let n = number(n);
            Box::new(move |&x| x < n)
        }
        _ => panic!("unknown predicate {words:?}"),
    }
}

fn function(words: &[&str]) -> Box<dyn Fn(i64) -> i64> {
    match words {
        ["square"] => Box::new(|x| x * x),
        ["negate"] => Box::new(|x| -x),
        ["add", n] => {
            let n = number(n);
            Box::new(move |x| x + n)
        }
        ["mul", n] => {
            let n = number(n);
            Box::new(move |x| x * n)
        }
        _ => panic!("unknown function {words:?}"),
    }
}

// Each stage wraps the iterator it is given in a new lazy one.
fn stage(items: Items, words: &[&str]) -> Items {
    let count = |n: &str| n.parse::<usize>().expect("a count");
    match words {
        ["map", rest @ ..] => Box::new(items.map(function(rest))),
        ["filter", rest @ ..] => Box::new(items.filter(predicate(rest))),
        ["takewhile", rest @ ..] => Box::new(items.take_while(predicate(rest))),
        ["dropwhile", rest @ ..] => Box::new(items.skip_while(predicate(rest))),
        ["take", n] => Box::new(items.take(count(n))),
        ["skip", n] => Box::new(items.skip(count(n))),
        ["step", n] => Box::new(items.step_by(count(n))),
        _ => panic!("unknown stage {words:?}"),
    }
}

fn run(line: &str) -> String {
    let mut parts: Vec<Vec<&str>> = line
        .split('|')
        .map(|part| part.split_whitespace().collect())
        .collect();
    // A sum or count at the end is not a stage: it says what to do with the items.
    let finish = match parts.last().map(Vec::as_slice) {
        Some(["sum" | "count"]) => parts.pop().expect("a last part")[0],
        _ => "list",
    };
    let mut items = source(&parts[0]);
    for words in &parts[1..] {
        items = stage(items, words);
    }
    // Nothing has run yet. Only the lines below pull items through.
    match finish {
        "sum" => items.sum::<i64>().to_string(),
        "count" => items.count().to_string(),
        _ => {
            let texts: Vec<String> = items.map(|item| item.to_string()).collect();
            if texts.is_empty() {
                "(empty)".to_string()
            } else {
                texts.join(" ")
            }
        }
    }
}

fn main() {
    for line in io::stdin().lock().lines() {
        let line = line.expect("readable input");
        if !line.trim().is_empty() {
            println!("{}", run(&line));
        }
    }
}
