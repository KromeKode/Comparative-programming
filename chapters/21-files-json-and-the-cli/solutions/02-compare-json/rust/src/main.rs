//! Prints how one JSON file differs from another: what was removed, what was added, and
//! what changed. ../README.md says what it should print.
use serde_json::Value;
use std::collections::BTreeSet;
use std::io::{self, ErrorKind, Write};
use std::process::ExitCode;

const USAGE: &str = "usage: jsondiff OLD NEW";

/// What strerror says for the errors the README names. Rust's own messages add the
/// number, as in "No such file or directory (os error 2)".
fn message(error: &io::Error) -> String {
    match error.kind() {
        ErrorKind::NotFound => "No such file or directory".to_string(),
        ErrorKind::IsADirectory => "Is a directory".to_string(),
        _ => error.to_string(),
    }
}

/// The value in a file, or the line that says why there isn't one.
fn read_json(path: &str) -> Result<Value, String> {
    let data = std::fs::read(path).map_err(|error| format!("{path}: {}", message(&error)))?;
    // from_slice checks that the bytes are UTF-8, and one JSON value with nothing after.
    serde_json::from_slice(&data).map_err(|_| format!("{path}: not valid JSON"))
}

/// Both files' values. ? returns the first problem, so NEW is read only if OLD was.
fn read_both(old_path: &str, new_path: &str) -> Result<(Value, Value), String> {
    Ok((read_json(old_path)?, read_json(new_path)?))
}

/// The part of a path that leads to a key: .name, or ["first name"].
fn step(key: &str) -> String {
    let mut chars = key.chars();
    let identifier = chars
        .next()
        .is_some_and(|c| c.is_ascii_alphabetic() || c == '_')
        && chars.all(|c| c.is_ascii_alphanumeric() || c == '_');
    if identifier {
        format!(".{key}")
    } else {
        format!("[{}]", Value::from(key)) // a Value's Display writes it as JSON
    }
}

/// Adds a line to out for each difference between before and after.
fn diff(path: &str, before: &Value, after: &Value, out: &mut Vec<String>) {
    match (before, after) {
        (Value::Object(before), Value::Object(after)) => {
            // A Map keeps its keys in order, as a BTreeMap does, and so does a BTreeSet.
            let keys: BTreeSet<&String> = before.keys().chain(after.keys()).collect();
            for key in keys {
                let here = path.to_string() + &step(key);
                match (before.get(key), after.get(key)) {
                    (Some(value), None) => out.push(format!("- {here}: {value}")),
                    (None, Some(value)) => out.push(format!("+ {here}: {value}")),
                    (Some(b), Some(a)) => diff(&here, b, a, out),
                    (None, None) => unreachable!("each key is in one of them"),
                }
            }
        }
        (Value::Array(before), Value::Array(after)) => {
            for i in 0..before.len().max(after.len()) {
                let here = format!("{path}[{i}]");
                match (before.get(i), after.get(i)) {
                    (Some(value), None) => out.push(format!("- {here}: {value}")),
                    (None, Some(value)) => out.push(format!("+ {here}: {value}")),
                    (Some(b), Some(a)) => diff(&here, b, a, out),
                    (None, None) => unreachable!("i is below one of the lengths"),
                }
            }
        }
        // == on two Values compares their kinds as well as what they hold.
        _ if before != after => out.push(format!("~ {path}: {before} -> {after}")),
        _ => {}
    }
}

fn main() -> ExitCode {
    let args: Vec<String> = std::env::args().skip(1).collect();
    let [old_path, new_path] = args.as_slice() else {
        eprintln!("{USAGE}");
        return ExitCode::from(2);
    };
    let (old, new) = match read_both(old_path, new_path) {
        Ok(values) => values,
        Err(problem) => {
            eprintln!("jsondiff: {problem}");
            return ExitCode::from(2);
        }
    };
    let mut lines = Vec::new();
    diff("$", &old, &new, &mut lines);
    let mut out = io::stdout().lock();
    for line in &lines {
        // A write fails if stdout is closed, as when the output goes to `head`.
        if writeln!(out, "{line}").is_err() {
            return ExitCode::from(2);
        }
    }
    ExitCode::from(if lines.is_empty() { 0 } else { 1 })
}
