//! Reads a web server's log, one request on each line, such as
//!   2026-11-23 14:05:12 GET /books/42 200 12ms
//! and prints, for each method and route, how many requests there were, how many failed,
//! and the slowest. A route is the path with each "/" and the digits after it written
//! "/:id", so that /books/42 and /books/7 count together. A line that isn't a request
//! gets a message on stderr, and the exit status is then 1.
use std::collections::HashMap;
use std::io::{self, Read};
use std::process::ExitCode;

use regex::Regex;

#[derive(Default)]
struct Route {
    requests: u32,
    failed: u32,
    slowest: u32,
}

fn main() -> ExitCode {
    // ^ and $, because captures finds a match anywhere in the line. Without the m flag,
    // $ matches only at the very end. [0-9], not \d: here \d matches any Unicode digit.
    let line_re = Regex::new(concat!(
        r"^(?<date>[0-9]{4}-[0-9]{2}-[0-9]{2}) (?<time>[0-9]{2}:[0-9]{2}:[0-9]{2})",
        r" (?<method>GET|POST|PUT|DELETE) (?<path>/[^ ]*)",
        r" (?<status>[0-9]{3}) (?<ms>[0-9]{1,9})ms$",
    ))
    .unwrap();
    let id = Regex::new("/[0-9]+").unwrap();

    let mut input = String::new();
    if let Err(err) = io::stdin().read_to_string(&mut input) {
        eprintln!("logscan: {err}");
        return ExitCode::FAILURE;
    }
    // A HashMap has no order, so the routes are kept in a Vec, with an index to find them.
    let mut routes: Vec<(String, Route)> = Vec::new();
    let mut index: HashMap<String, usize> = HashMap::new();
    let mut status = ExitCode::SUCCESS;
    // lines() splits at "\n" and removes a "\r" before it.
    for (number, line) in input.lines().enumerate() {
        let Some(caps) = line_re.captures(line) else {
            eprintln!("logscan: line {}: not a log line", number + 1);
            status = ExitCode::FAILURE;
            continue;
        };
        let path = id.replace_all(&caps["path"], "/:id");
        let key = format!("{} {path}", &caps["method"]);
        let at = *index.entry(key.clone()).or_insert_with(|| {
            routes.push((key, Route::default()));
            routes.len() - 1
        });
        let route = &mut routes[at].1;
        // The regex let through only digits, few enough to fit, so parse can't fail.
        let code: u32 = caps["status"].parse().unwrap();
        let ms: u32 = caps["ms"].parse().unwrap();
        route.requests += 1;
        route.failed += u32::from(code >= 400);
        route.slowest = route.slowest.max(ms);
    }
    for (key, route) in &routes {
        let plural = if route.requests == 1 { "" } else { "s" };
        println!(
            "{key}: {} request{plural}, {} failed, slowest {}ms",
            route.requests, route.failed, route.slowest
        );
    }
    status
}
