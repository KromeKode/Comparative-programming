//! Prints the lines that contain a pattern, from each file named on the command line, or
//! from stdin. ../README.md says what it should print.
use std::fs;
use std::io::{self, BufWriter, ErrorKind, Read, Write};
use std::process::ExitCode;

const USAGE: &str = "usage: search [-icnv] PATTERN [FILE...]";
const STDIN_NAME: &str = "(standard input)";

#[derive(Default)]
struct Options {
    ignore_case: bool,
    count: bool,
    numbers: bool,
    invert: bool,
}

fn usage_error(message: &str) -> ExitCode {
    eprintln!("search: {message}\n{USAGE}");
    ExitCode::from(2)
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

/// Whether needle is somewhere in haystack. windows(n) gives every run of n bytes.
fn contains(haystack: &[u8], needle: &[u8]) -> bool {
    needle.is_empty() || haystack.windows(needle.len()).any(|run| run == needle)
}

/// Writes the lines of data that are selected, and returns how many there are.
fn search(
    out: &mut impl Write,
    data: &[u8],
    name: Option<&str>,
    pattern: &[u8],
    options: &Options,
) -> io::Result<usize> {
    // to_ascii_lowercase changes A to Z only, and leaves every other byte as it is.
    let pattern = if options.ignore_case {
        pattern.to_ascii_lowercase()
    } else {
        pattern.to_vec()
    };
    let prefix = name.map(|name| format!("{name}:")).unwrap_or_default();
    let mut lines: Vec<&[u8]> = data.split(|&byte| byte == b'\n').collect();
    if lines.last() == Some(&&b""[..]) {
        lines.pop(); // the data ended with a newline, or was empty
    }
    let mut count = 0;
    for (i, line) in lines.into_iter().enumerate() {
        let text = if options.ignore_case {
            line.to_ascii_lowercase()
        } else {
            line.to_vec()
        };
        let found = contains(&text, &pattern);
        if found == options.invert {
            continue;
        }
        count += 1;
        if !options.count {
            let number = if options.numbers {
                format!("{}:", i + 1)
            } else {
                String::new()
            };
            out.write_all(format!("{prefix}{number}").as_bytes())?;
            out.write_all(line)?;
            out.write_all(b"\n")?;
        }
    }
    if options.count {
        writeln!(out, "{prefix}{count}")?;
    }
    Ok(count)
}

fn main() -> ExitCode {
    let args: Vec<String> = std::env::args().skip(1).collect();
    let mut options = Options::default();
    let mut rest = args.as_slice();
    // Options come first; the first argument that isn't one is the pattern.
    while let [arg, tail @ ..] = rest {
        if !arg.starts_with('-') || arg == "-" {
            break;
        }
        rest = tail;
        if arg == "--" {
            break;
        }
        for letter in arg[1..].chars() {
            match letter {
                'i' => options.ignore_case = true,
                'c' => options.count = true,
                'n' => options.numbers = true,
                'v' => options.invert = true,
                _ => return usage_error(&format!("unknown option -{letter}")),
            }
        }
    }
    let [pattern, files @ ..] = rest else {
        return usage_error("no pattern");
    };
    let files: Vec<&str> = if files.is_empty() {
        vec!["-"]
    } else {
        files.iter().map(String::as_str).collect()
    };

    // Stdout is line-buffered, a write to the system at each "\n"; BufWriter saves them
    // up. Writing to a closed pipe is an error, which ends the program here.
    let mut out = BufWriter::new(io::stdout().lock());
    let mut selected = 0;
    let mut status = 0;
    for &file in &files {
        let data = if file == "-" {
            let mut data = Vec::new();
            io::stdin().read_to_end(&mut data).map(|_| data)
        } else {
            fs::read(file) // bytes, whether or not they are UTF-8
        };
        let data = match data {
            Ok(data) => data,
            Err(error) => {
                let _ = out.flush(); // what came before, before the error
                eprintln!("search: {file}: {}", message(&error));
                status = 2;
                continue;
            }
        };
        let name = (files.len() > 1).then_some(if file == "-" { STDIN_NAME } else { file });
        match search(&mut out, &data, name, pattern.as_bytes(), &options) {
            Ok(count) => selected += count,
            Err(_) => return ExitCode::from(2),
        }
    }
    if out.flush().is_err() {
        return ExitCode::from(2);
    }
    if status == 0 && selected == 0 {
        status = 1;
    }
    ExitCode::from(status)
}
