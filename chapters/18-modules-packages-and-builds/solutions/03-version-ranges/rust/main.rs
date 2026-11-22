// Exercise 18.3: version ranges. The highest version that each range allows.
//
// The versions and ranges are semver.rs's business; this file reads the input and
// prints the answers.
mod semver;

use semver::{Range, Version};
use std::io::{self, Read};

fn main() {
    let mut input = String::new();
    io::stdin()
        .read_to_string(&mut input)
        .expect("can't read stdin");
    let mut lines = input.lines();
    let Some(first) = lines.next() else {
        return;
    };
    let mut versions = Vec::new();
    for word in first.split_whitespace() {
        match Version::parse(word) {
            Some(version) => versions.push(version),
            None => {
                println!("error: line 1: {word} isn't a version");
                return;
            }
        }
    }
    if versions.is_empty() {
        println!("error: line 1 must list the versions");
        return;
    }
    for (number, raw) in (2..).zip(lines) {
        let line = raw.trim();
        if line.is_empty() {
            continue;
        }
        match Range::parse(line) {
            Ok(range) => match range.highest(&versions) {
                Some(best) => println!("{line} -> {best}"),
                None => println!("{line} -> none"),
            },
            Err(error) => println!("error: line {number}: {error}"),
        }
    }
}
