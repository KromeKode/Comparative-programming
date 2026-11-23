//! Reads CSV, as RFC 4180 describes it, and prints each record as a JSON list of strings,
//! one record on each line. A field in quotes can hold commas, line breaks, and "" for a
//! quote. Lines can end with "\r\n" or "\n". Every record must have as many fields as the
//! first. At the first error, it prints the line and column, and the exit status is 1.
use std::fmt;
use std::io::{self, BufWriter, Read, Write};
use std::process::ExitCode;

#[derive(Clone, Copy, PartialEq)]
enum State {
    RecordStart, // nothing of this record read yet
    FieldStart,  // just after a comma
    Unquoted,    // in a field that doesn't start with a quote
    Quoted,      // in a field that does
    Quote,       // just after a quote in a quoted field: its end, or the first of ""
    Cr,          // just after a "\r" outside quotes, which needs a "\n" next
}

struct CsvError {
    line: usize,
    column: usize,
    message: String,
}

impl fmt::Display for CsvError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(
            f,
            "line {}, column {}: {}",
            self.line, self.column, self.message
        )
    }
}

fn error(line: usize, column: usize, message: &str) -> Result<(), CsvError> {
    let message = message.to_string();
    Err(CsvError {
        line,
        column,
        message,
    })
}

fn fields(n: usize) -> String {
    if n == 1 {
        "1 field".to_string()
    } else {
        format!("{n} fields")
    }
}

/// Calls emit with each record in text, and returns the first error.
fn parse(text: &str, mut emit: impl FnMut(&[String])) -> Result<(), CsvError> {
    use State::*;
    let mut state = RecordStart;
    let mut record: Vec<String> = Vec::new();
    let mut field = String::new();
    let mut width = 0; // how many fields the first record has, once it has ended
    let (mut line, mut column) = (1, 0); // where char is: columns count characters, from 1
    let mut start = (0, 0); // where the quote that began a quoted field is, or the "\r"
    let mut record_line = 1;
    // chars() goes through the text's chars, which are code points, so "🦊" is one. The
    // None after them stands for the end, so that the end goes through the machine too.
    for char in text.chars().map(Some).chain([None]) {
        column += 1;
        if state == RecordStart {
            record_line = line;
        }
        let mut ended = false; // whether char ends a record
        // Each arm is a state and a character; the first that fits is taken.
        match (state, char) {
            (Quoted, None) => return error(start.0, start.1, "quoted field is never closed"),
            (Quoted, Some('"')) => state = Quote,
            (Quoted, Some(c)) => field.push(c),
            (Quote, Some('"')) => {
                field.push('"');
                state = Quoted;
            }
            (Cr, Some('\n')) => ended = true,
            (Cr, _) => {
                return error(
                    start.0,
                    start.1,
                    "a carriage return without a line feed after it",
                );
            }
            (RecordStart, None) => {}
            (RecordStart | FieldStart, Some('"')) => {
                start = (line, column);
                state = Quoted;
            }
            (Unquoted, Some('"')) => return error(line, column, "a quote in an unquoted field"),
            // From here on, the state is RecordStart, FieldStart, Unquoted, or Quote.
            (_, Some(',')) => {
                record.push(std::mem::take(&mut field));
                state = FieldStart;
            }
            (_, Some('\n') | None) => ended = true,
            (_, Some('\r')) => {
                start = (line, column);
                state = Cr;
            }
            (Quote, Some(_)) => {
                return error(
                    line,
                    column,
                    "expected a comma or a line end after a closing quote",
                );
            }
            (RecordStart | FieldStart | Unquoted, Some(c)) => {
                field.push(c);
                state = Unquoted;
            }
        }
        if ended {
            record.push(std::mem::take(&mut field));
            if width == 0 {
                width = record.len();
            } else if record.len() != width {
                let n = fields(record.len());
                return error(
                    record_line,
                    1,
                    &format!("record has {n}, but the first has {width}"),
                );
            }
            emit(&record);
            record.clear();
            state = RecordStart;
        }
        if char == Some('\n') {
            (line, column) = (line + 1, 0);
        }
    }
    Ok(())
}

/// Writes s as a JSON string, as JavaScript's JSON.stringify does.
fn write_string(out: &mut impl Write, s: &str) -> io::Result<()> {
    out.write_all(b"\"")?;
    for c in s.chars() {
        match c {
            '"' => out.write_all(b"\\\"")?,
            '\\' => out.write_all(b"\\\\")?,
            '\u{8}' => out.write_all(b"\\b")?,
            '\u{c}' => out.write_all(b"\\f")?,
            '\n' => out.write_all(b"\\n")?,
            '\r' => out.write_all(b"\\r")?,
            '\t' => out.write_all(b"\\t")?,
            c if c < ' ' => write!(out, "\\u{:04x}", c as u32)?,
            c => write!(out, "{c}")?,
        }
    }
    out.write_all(b"\"")
}

/// Writes record as a JSON list of strings, on a line of its own.
fn write_record(out: &mut impl Write, record: &[String]) -> io::Result<()> {
    out.write_all(b"[")?;
    for (i, field) in record.iter().enumerate() {
        if i > 0 {
            out.write_all(b",")?;
        }
        write_string(out, field)?;
    }
    out.write_all(b"]\n")
}

fn main() -> ExitCode {
    let mut text = String::new();
    if let Err(err) = io::stdin().read_to_string(&mut text) {
        eprintln!("csv2json: {err}");
        return ExitCode::FAILURE;
    }
    // Rust's stdout writes each line as it ends; a BufWriter saves them up.
    let mut out = BufWriter::new(io::stdout().lock());
    let result = parse(&text, |record| {
        write_record(&mut out, record).expect("can't write to stdout");
    });
    out.flush().expect("can't write to stdout");
    match result {
        Ok(()) => ExitCode::SUCCESS,
        Err(err) => {
            eprintln!("csv2json: {err}");
            ExitCode::FAILURE
        }
    }
}
