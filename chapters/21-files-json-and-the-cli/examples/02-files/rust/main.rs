//! Removes repeated lines from each file named on the command line, keeping the first of
//! each, and rewrites the file in place. It writes a new file next to the old one and
//! renames it over the old one, so that a crash halfway leaves the old file whole.
use std::collections::HashSet;
use std::fs::{self, OpenOptions};
use std::io::{self, Write};
use std::process::ExitCode;

/// Rewrites the file without its repeated lines, and returns how many there were.
fn dedup(path: &str) -> io::Result<usize> {
    // read_to_string fails if the bytes aren't UTF-8.
    let text = fs::read_to_string(path)?;
    // split_terminator leaves out the empty string after a final newline. lines() would
    // also take the "\r" off the end of each line.
    let lines: Vec<&str> = text.split_terminator('\n').collect();
    let mut seen = HashSet::new();
    // insert returns false for a line it has seen, so filter keeps the first of each.
    let kept: Vec<&str> = lines
        .iter()
        .copied()
        .filter(|line| seen.insert(*line))
        .collect();
    let removed = lines.len() - kept.len();
    if removed == 0 {
        return Ok(0);
    }

    let permissions = fs::metadata(path)?.permissions();
    let new_text: String = kept.iter().map(|line| format!("{line}\n")).collect();
    // The standard library can't make a file with a fresh name, so the new file is
    // FILE.tmp, and create_new fails if that is already there rather than writing over it.
    let temporary = format!("{path}.tmp");
    let mut file = OpenOptions::new()
        .write(true)
        .create_new(true)
        .open(&temporary)?;
    let written = file
        .write_all(new_text.as_bytes())
        .and_then(|()| file.set_permissions(permissions)) // the old file's permissions
        .and_then(|()| file.sync_all()) // onto the disk before the rename
        .and_then(|()| fs::rename(&temporary, path));
    if let Err(error) = written {
        let _ = fs::remove_file(&temporary); // an error here would hide the first one
        return Err(error);
    }
    Ok(removed)
}

fn main() -> ExitCode {
    let paths: Vec<String> = std::env::args().skip(1).collect();
    if paths.is_empty() {
        eprintln!("usage: dedup FILE...");
        return ExitCode::from(2);
    }
    let mut status = ExitCode::SUCCESS;
    for path in &paths {
        match dedup(path) {
            Ok(removed) => {
                let plural = if removed == 1 { "" } else { "s" };
                println!("{path}: removed {removed} line{plural}");
            }
            Err(error) => {
                // Such as "No such file or directory (os error 2)": no path, so it's added.
                eprintln!("dedup: {path}: {error}");
                status = ExitCode::FAILURE;
            }
        }
    }
    status
}
