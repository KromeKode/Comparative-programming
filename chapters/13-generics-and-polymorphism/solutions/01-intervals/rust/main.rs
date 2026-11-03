use std::fmt::{self, Display};
use std::io::{self, BufRead};

// derive(Ord) compares the fields in order, which is the right order here:
// major first, then minor, then patch, each as a number.
#[derive(PartialEq, Eq, PartialOrd, Ord)]
struct Version {
    major: u64,
    minor: u64,
    patch: u64,
}

impl Display for Version {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}.{}.{}", self.major, self.minor, self.patch)
    }
}

struct Interval<T> {
    low: T,
    high: T,
}

impl<T: Display> Display for Interval<T> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}..{}", self.low, self.high)
    }
}

/// Sorts by the low ends, then joins each interval to the last one if they overlap.
fn merge<T: Ord>(mut intervals: Vec<Interval<T>>) -> Vec<Interval<T>> {
    intervals.sort_by(|a, b| a.low.cmp(&b.low));
    let mut merged: Vec<Interval<T>> = Vec::new();
    for interval in intervals {
        match merged.last_mut() {
            Some(last) if interval.low <= last.high => {
                if interval.high > last.high {
                    last.high = interval.high;
                }
            }
            _ => merged.push(interval),
        }
    }
    merged
}

fn is_digits(text: &str) -> bool {
    !text.is_empty() && text.bytes().all(|b| b.is_ascii_digit())
}

fn parse_int(text: &str) -> Option<i64> {
    // str::parse would also take "+5", so check the form first.
    let digits = text.strip_prefix('-').unwrap_or(text);
    if !is_digits(digits) {
        return None;
    }
    text.parse().ok()
}

fn parse_word(text: &str) -> Option<String> {
    let letters = !text.is_empty() && text.bytes().all(|b| b.is_ascii_lowercase());
    letters.then(|| text.to_string())
}

fn parse_version(text: &str) -> Option<Version> {
    let parts: Vec<&str> = text.split('.').collect();
    let [major, minor, patch] = parts[..] else {
        return None;
    };
    if !parts.iter().all(|part| is_digits(part)) {
        return None;
    }
    Some(Version {
        major: major.parse().ok()?,
        minor: minor.parse().ok()?,
        patch: patch.parse().ok()?,
    })
}

/// Parses every interval on a line with parse, merges them, and formats the result.
fn run<T: Ord + Display>(
    kind: &str,
    parse: fn(&str) -> Option<T>,
    words: &[&str],
) -> Result<String, String> {
    let mut intervals = Vec::new();
    for word in words {
        let ends: Vec<&str> = word.split("..").collect();
        let [low_text, high_text] = ends[..] else {
            return Err(format!("bad interval \"{word}\""));
        };
        let low = parse(low_text).ok_or_else(|| format!("bad {kind} \"{low_text}\""))?;
        let high = parse(high_text).ok_or_else(|| format!("bad {kind} \"{high_text}\""))?;
        if high < low {
            return Err(format!("{word} is backwards"));
        }
        intervals.push(Interval { low, high });
    }
    let mut parts = vec![kind.to_string()];
    parts.extend(merge(intervals).iter().map(|interval| interval.to_string()));
    Ok(parts.join(" "))
}

fn main() {
    for line in io::stdin().lock().lines() {
        let line = line.expect("read stdin");
        let words: Vec<&str> = line.split_whitespace().collect();
        let Some((&kind, rest)) = words.split_first() else {
            continue;
        };
        // Each arm fixes T, so the compiler makes one copy of run for each kind.
        let result = match kind {
            "int" => run(kind, parse_int, rest),
            "word" => run(kind, parse_word, rest),
            "version" => run(kind, parse_version, rest),
            _ => Err(format!("unknown kind \"{kind}\"")),
        };
        match result {
            Ok(text) => println!("{text}"),
            Err(message) => println!("error: {message}"),
        }
    }
}
