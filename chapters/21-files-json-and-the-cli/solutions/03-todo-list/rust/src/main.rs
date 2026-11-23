//! Keeps a to-do list in todo.json, in the current folder: adds tasks, lists them, marks
//! them done, and removes them. ../README.md says what it should do.
use serde::{Deserialize, Serialize};
use serde_json::Value;
use std::collections::HashSet;
use std::fs::{self, File};
use std::io::{self, ErrorKind, Write};
use std::process::ExitCode;

const FILE: &str = "todo.json";
const VERSION: u64 = 1;
const USAGE: &str = "usage: todo add TEXT...
       todo list
       todo done ID
       todo remove ID";

// serde reads and writes the fields in this order, under these names. A missing field
// is an error, as none of them has a default, and deny_unknown_fields makes a key that
// no field has an error too, where serde would ignore it.
#[derive(Serialize, Deserialize)]
#[serde(deny_unknown_fields)]
struct Task {
    id: u64,
    text: String,
    done: bool,
}

#[derive(Serialize, Deserialize)]
#[serde(deny_unknown_fields)]
struct List {
    version: u64,
    tasks: Vec<Task>,
}

/// What strerror says for the errors the README names. Rust's own messages add the
/// number, as in "No such file or directory (os error 2)".
fn message(error: &io::Error) -> String {
    match error.kind() {
        ErrorKind::NotFound => "No such file or directory".to_string(),
        ErrorKind::IsADirectory => "Is a directory".to_string(),
        _ => error.to_string(),
    }
}

/// The list in todo.json, or an empty one if there's no todo.json, or what's wrong.
fn read_list() -> Result<List, String> {
    let data = match fs::read(FILE) {
        Ok(data) => data,
        Err(error) if error.kind() == ErrorKind::NotFound => {
            return Ok(List { version: VERSION, tasks: Vec::new() });
        }
        Err(error) => return Err(message(&error)),
    };
    let value: Value = serde_json::from_slice(&data).map_err(|_| "not valid JSON")?;
    // A later version of todo may keep anything else in the file, so the version comes
    // first. get gives None for a key that isn't there, or a value that isn't an object.
    if let Some(version) = value.get("version")
        && version.is_number()
        && version.as_u64() != Some(VERSION)
    {
        return Err(format!("unknown version {version}"));
    }
    let list: List = serde_json::from_value(value).map_err(|_| "not a to-do list")?;
    let mut ids = HashSet::new();
    // insert returns false for an id it has already.
    if list.tasks.iter().any(|task| task.id == 0 || !ids.insert(task.id)) {
        return Err("not a to-do list".to_string());
    }
    Ok(list)
}

/// Writes the list to todo.json. It writes a new file and renames it over the old one,
/// which replaces it in one step, so that a crash leaves the old list or the new one,
/// and never half of one.
fn write_list(list: &List) -> Result<(), String> {
    // to_string_pretty can only fail for a map whose keys aren't strings.
    let text = serde_json::to_string_pretty(list).expect("a list") + "\n";
    let temporary = format!("{FILE}.tmp");
    let written = File::create(&temporary).and_then(|mut file| {
        file.write_all(text.as_bytes())
            .and_then(|()| file.sync_all()) // onto the disk before the rename
            .and_then(|()| fs::rename(&temporary, FILE))
    });
    written.map_err(|error| {
        let _ = fs::remove_file(&temporary); // an error here would hide the first one
        message(&error)
    })
}

/// Carries out a command whose arguments have been checked, and returns the status.
fn run(command: &str, args: &[String]) -> Result<u8, String> {
    let mut list = read_list()?;
    if command == "add" {
        let id = list.tasks.iter().map(|task| task.id).max().unwrap_or(0) + 1;
        let text = args.join(" ");
        let line = format!("added {id}: {text}");
        list.tasks.push(Task { id, text, done: false });
        write_list(&list)?;
        println!("{line}");
        return Ok(0);
    }
    if command == "list" {
        for task in &list.tasks {
            let mark = if task.done { 'x' } else { ' ' };
            println!("{} [{mark}] {}", task.id, task.text);
        }
        return Ok(0);
    }
    // The ID is digits, so parse fails only if it's too big for a u64, as no id is.
    let id: u64 = args[0].parse().unwrap_or(u64::MAX);
    let Some(i) = list.tasks.iter().position(|task| task.id == id) else {
        eprintln!("todo: no task {}", args[0]);
        return Ok(1);
    };
    let line = if command == "done" {
        list.tasks[i].done = true;
        format!("done {id}: {}", list.tasks[i].text)
    } else {
        let task = list.tasks.remove(i);
        format!("removed {id}: {}", task.text)
    };
    write_list(&list)?;
    println!("{line}");
    Ok(0)
}

fn main() -> ExitCode {
    let args: Vec<String> = std::env::args().skip(1).collect();
    let (command, args) = args.split_first().map_or(("", &[][..]), |(c, a)| (c.as_str(), a));
    let is_id = |arg: &String| !arg.is_empty() && arg.bytes().all(|b| b.is_ascii_digit());
    // The command line is checked before todo.json is read.
    let ok = match command {
        "add" => !args.is_empty(),
        "list" => args.is_empty(),
        "done" | "remove" => args.len() == 1 && is_id(&args[0]),
        _ => false,
    };
    if !ok {
        eprintln!("{USAGE}");
        return ExitCode::from(2);
    }
    match run(command, args) {
        Ok(status) => ExitCode::from(status),
        Err(problem) => {
            eprintln!("todo: {FILE}: {problem}");
            ExitCode::from(2)
        }
    }
}
