//! Reads one JSON text on stdin, strictly as RFC 8259 has it, and prints it compactly:
//! no spaces, keys in the order they came, numbers as they were written, and strings
//! written again with only the escapes they need. Or prints the first error's line and
//! column, and exits with 1.
use std::collections::HashSet;
use std::io::{self, Read};
use std::process::ExitCode;

const MAX_DEPTH: usize = 1000;

/// s as a JSON string, with only the escapes it needs.
fn quote(s: &str) -> String {
    let mut out = String::from("\"");
    for c in s.chars() {
        match c {
            '"' => out.push_str("\\\""),
            '\\' => out.push_str("\\\\"),
            '\u{8}' => out.push_str("\\b"),
            '\u{c}' => out.push_str("\\f"),
            '\n' => out.push_str("\\n"),
            '\r' => out.push_str("\\r"),
            '\t' => out.push_str("\\t"),
            c if c < ' ' => out.push_str(&format!("\\u{:04x}", c as u32)),
            c => out.push(c),
        }
    }
    out.push('"');
    out
}

struct JsonError {
    at: usize, // an index into the chars
    message: String,
}

type Result<T> = std::result::Result<T, JsonError>;

struct Parser {
    text: Vec<char>, // chars, so that an index counts code points, as columns do
    i: usize,
    depth: usize,
}

impl Parser {
    fn peek(&self) -> Option<char> {
        self.text.get(self.i).copied()
    }

    fn error<T>(&self, at: usize, message: impl Into<String>) -> Result<T> {
        Err(JsonError {
            at,
            message: message.into(),
        })
    }

    fn fail<T>(&self, expected: &str) -> Result<T> {
        let found = match self.peek() {
            Some(c) => quote(&c.to_string()),
            None => "the end of the input".to_string(),
        };
        self.error(self.i, format!("expected {expected}, found {found}"))
    }

    fn space(&mut self) {
        while matches!(self.peek(), Some(' ' | '\t' | '\n' | '\r')) {
            self.i += 1;
        }
    }

    fn starts_with(&self, at: usize, word: &str) -> bool {
        word.chars()
            .enumerate()
            .all(|(k, c)| self.text.get(at + k) == Some(&c))
    }

    fn value(&mut self) -> Result<String> {
        match self.peek() {
            Some(c @ ('{' | '[')) => {
                self.depth += 1;
                if self.depth > MAX_DEPTH {
                    return self.error(self.i, format!("nesting deeper than {MAX_DEPTH}"));
                }
                let out = if c == '{' {
                    self.object()?
                } else {
                    self.array()?
                };
                self.depth -= 1;
                Ok(out)
            }
            Some('"') => Ok(quote(&self.string()?)),
            Some('-' | '0'..='9') => self.number(),
            _ => {
                for word in ["true", "false", "null"] {
                    if self.starts_with(self.i, word) {
                        self.i += word.len();
                        return Ok(word.to_string());
                    }
                }
                self.fail("a value")
            }
        }
    }

    fn object(&mut self) -> Result<String> {
        self.i += 1; // the {
        self.space();
        if self.peek() == Some('}') {
            self.i += 1;
            return Ok("{}".to_string());
        }
        let mut members = Vec::new();
        let mut keys = HashSet::new();
        loop {
            if self.peek() != Some('"') {
                let first = members.is_empty();
                return self.fail(if first {
                    "a string key or \"}\""
                } else {
                    "a string key"
                });
            }
            let at = self.i;
            let key = self.string()?;
            if keys.contains(&key) {
                return self.error(at, format!("duplicate key {}", quote(&key)));
            }
            self.space();
            if self.peek() != Some(':') {
                return self.fail("\":\"");
            }
            self.i += 1;
            self.space();
            members.push(format!("{}:{}", quote(&key), self.value()?));
            keys.insert(key);
            self.space();
            match self.peek() {
                Some('}') => {
                    self.i += 1;
                    return Ok(format!("{{{}}}", members.join(",")));
                }
                Some(',') => {
                    self.i += 1;
                    self.space();
                }
                _ => return self.fail("\",\" or \"}\""),
            }
        }
    }

    fn array(&mut self) -> Result<String> {
        self.i += 1; // the [
        self.space();
        if self.peek() == Some(']') {
            self.i += 1;
            return Ok("[]".to_string());
        }
        let mut items = Vec::new();
        loop {
            items.push(self.value()?);
            self.space();
            match self.peek() {
                Some(']') => {
                    self.i += 1;
                    return Ok(format!("[{}]", items.join(",")));
                }
                Some(',') => {
                    self.i += 1;
                    self.space();
                }
                _ => return self.fail("\",\" or \"]\""),
            }
        }
    }

    fn digits(&mut self) -> Result<()> {
        if !matches!(self.peek(), Some('0'..='9')) {
            return self.fail("a digit");
        }
        while matches!(self.peek(), Some('0'..='9')) {
            self.i += 1;
        }
        Ok(())
    }

    fn number(&mut self) -> Result<String> {
        let start = self.i;
        if self.peek() == Some('-') {
            self.i += 1;
        }
        if self.peek() == Some('0') {
            self.i += 1; // a 0 can't have digits after it
        } else {
            self.digits()?;
        }
        if self.peek() == Some('.') {
            self.i += 1;
            self.digits()?;
        }
        if matches!(self.peek(), Some('e' | 'E')) {
            self.i += 1;
            if matches!(self.peek(), Some('+' | '-')) {
                self.i += 1;
            }
            self.digits()?;
        }
        Ok(self.text[start..self.i].iter().collect()) // as it was written
    }

    /// The four hex digits at text[at..], or None if they aren't.
    fn hex4(&self, at: usize) -> Option<u32> {
        let digits = self.text.get(at..at + 4)?;
        digits
            .iter()
            .try_fold(0, |n, c| Some(n * 16 + c.to_digit(16)?))
    }

    /// The string at i, with its escapes undone.
    fn string(&mut self) -> Result<String> {
        let start = self.i;
        self.i += 1; // the "
        let mut out = String::new();
        loop {
            let Some(c) = self.peek() else {
                return self.error(start, "string is never closed");
            };
            match c {
                '"' => {
                    self.i += 1;
                    return Ok(out);
                }
                c if c < ' ' => return self.error(self.i, "control character in a string"),
                '\\' => {}
                c => {
                    out.push(c);
                    self.i += 1;
                    continue;
                }
            }
            let simple = match self.text.get(self.i + 1) {
                None => return self.error(start, "string is never closed"),
                Some('u') => None, // four hex digits, below
                Some('"') => Some('"'),
                Some('\\') => Some('\\'),
                Some('/') => Some('/'),
                Some('b') => Some('\u{8}'),
                Some('f') => Some('\u{c}'),
                Some('n') => Some('\n'),
                Some('r') => Some('\r'),
                Some('t') => Some('\t'),
                Some(_) => return self.error(self.i, "invalid escape"),
            };
            if let Some(c) = simple {
                out.push(c);
                self.i += 2;
                continue;
            }
            let Some(mut code) = self.hex4(self.i + 2) else {
                return self.error(self.i, "invalid escape");
            };
            let at = self.i;
            self.i += 6;
            if (0xDC00..=0xDFFF).contains(&code) {
                return self.error(at, "unpaired surrogate");
            }
            if (0xD800..=0xDBFF).contains(&code) {
                // A character above U+FFFF is two escapes, a high surrogate and a low one.
                let low = if self.starts_with(self.i, "\\u") {
                    self.hex4(self.i + 2)
                } else {
                    None
                };
                match low {
                    Some(low @ 0xDC00..=0xDFFF) => {
                        code = 0x10000 + ((code - 0xD800) << 10) + (low - 0xDC00);
                        self.i += 6;
                    }
                    _ => return self.error(at, "unpaired surrogate"),
                }
            }
            // Not a surrogate now, so from_u32 can't fail.
            out.push(char::from_u32(code).unwrap());
        }
    }

    fn parse(&mut self) -> Result<String> {
        self.space();
        let out = self.value()?;
        self.space();
        if self.i < self.text.len() {
            return self.fail("the end of the input");
        }
        Ok(out)
    }
}

fn main() -> ExitCode {
    let mut text = String::new();
    if let Err(err) = io::stdin().read_to_string(&mut text) {
        eprintln!("jsoncheck: {err}");
        return ExitCode::FAILURE;
    }
    let mut parser = Parser {
        text: text.chars().collect(),
        i: 0,
        depth: 0,
    };
    match parser.parse() {
        Ok(out) => {
            println!("{out}");
            ExitCode::SUCCESS
        }
        Err(err) => {
            // Lines end at "\n", and columns count characters from 1.
            let before = &parser.text[..err.at];
            let line = before.iter().filter(|&&c| c == '\n').count() + 1;
            let column = before.iter().rev().take_while(|&&c| c != '\n').count() + 1;
            eprintln!("jsoncheck: line {line}, column {column}: {}", err.message);
            ExitCode::FAILURE
        }
    }
}
