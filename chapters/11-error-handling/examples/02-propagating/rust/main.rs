use std::error::Error;
use std::fmt;

// Pretend files, so the example does not touch the disk.
const FILES: [(&str, &str); 2] = [("good.txt", "1\n2\n3"), ("bad.txt", "4\nfive\n6")];

#[derive(Debug)]
struct NotFound(String);

impl fmt::Display for NotFound {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}: no such file", self.0)
    }
}

impl Error for NotFound {}

#[derive(Debug)]
struct NotANumber(String);

impl fmt::Display for NotANumber {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "\"{}\" is not a number", self.0)
    }
}

impl Error for NotANumber {}

// Everything sum_file can fail with, as one enum.
#[derive(Debug)]
enum SumError {
    NotFound(NotFound),
    BadLine {
        file: String,
        line: usize,
        source: NotANumber,
    },
}

impl fmt::Display for SumError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            SumError::NotFound(error) => write!(f, "{error}"),
            SumError::BadLine { file, line, .. } => write!(f, "{file}: line {line}"),
        }
    }
}

impl Error for SumError {
    // The error that caused this one, if there is one.
    fn source(&self) -> Option<&(dyn Error + 'static)> {
        match self {
            SumError::NotFound(_) => None,
            SumError::BadLine { source, .. } => Some(source),
        }
    }
}

// This lets ? turn a NotFound into a SumError on its own.
impl From<NotFound> for SumError {
    fn from(error: NotFound) -> Self {
        SumError::NotFound(error)
    }
}

fn read_file(name: &str) -> Result<&'static str, NotFound> {
    FILES
        .iter()
        .find(|(file, _)| *file == name)
        .map(|(_, text)| *text)
        .ok_or_else(|| NotFound(name.to_string()))
}

fn parse_number(text: &str) -> Result<i64, NotANumber> {
    text.parse().map_err(|_| NotANumber(text.to_string()))
}

fn sum_file(name: &str) -> Result<i64, SumError> {
    let text = read_file(name)?; // on Err, convert it and return it
    let mut total = 0;
    for (index, text) in text.lines().enumerate() {
        total += parse_number(text).map_err(|source| SumError::BadLine {
            file: name.to_string(),
            line: index + 1,
            source,
        })?;
    }
    Ok(total)
}

// A missing file counts as 0, but any other error stops the sum.
fn sum_all(names: &[&str]) -> Result<i64, SumError> {
    let mut total = 0;
    for name in names {
        match sum_file(name) {
            Ok(value) => total += value,
            Err(SumError::NotFound(_)) => {}
            Err(other) => return Err(other),
        }
    }
    Ok(total)
}

// Join an error's message with the messages of its sources.
fn describe(error: &dyn Error) -> String {
    let mut text = error.to_string();
    let mut cause = error.source();
    while let Some(inner) = cause {
        text += ": ";
        text += &inner.to_string();
        cause = inner.source();
    }
    text
}

fn show(label: &str, result: Result<i64, SumError>) {
    match result {
        Ok(total) => println!("  {label} -> {total}"),
        Err(error) => println!("  {label} -> error: {}", describe(&error)),
    }
}

fn main() {
    println!("one file at a time:");
    for name in ["good.txt", "bad.txt", "missing.txt"] {
        show(name, sum_file(name));
    }
    println!("several files, where a missing file counts as 0:");
    for names in [["good.txt", "missing.txt"], ["good.txt", "bad.txt"]] {
        show(&names.join(" + "), sum_all(&names));
    }
}
