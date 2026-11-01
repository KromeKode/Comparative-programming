use std::fmt;
use std::io::{self, BufRead};

#[derive(Debug, PartialEq)]
enum ParseError {
    Empty,
    NoDigits,
    InvalidCharacter { character: char, position: usize },
    TooLarge,
    TooSmall,
}

impl fmt::Display for ParseError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            ParseError::Empty => write!(f, "empty input"),
            ParseError::NoDigits => write!(f, "no digits after the sign"),
            ParseError::InvalidCharacter {
                character,
                position,
            } => write!(f, "invalid character '{character}' at position {position}"),
            ParseError::TooLarge => write!(f, "too large, the maximum is {}", i32::MAX),
            ParseError::TooSmall => write!(f, "too small, the minimum is {}", i32::MIN),
        }
    }
}

impl std::error::Error for ParseError {}

fn parse_int(text: &str) -> Result<i32, ParseError> {
    let (negative, digits) = match text.as_bytes().first() {
        None => return Err(ParseError::Empty),
        Some(b'-') => (true, &text[1..]),
        Some(b'+') => (false, &text[1..]),
        Some(_) => (false, text),
    };
    if digits.is_empty() {
        return Err(ParseError::NoDigits);
    }
    let start = text.len() - digits.len();
    // Check every character first, so a bad one is reported even when the
    // digits before it are already too large.
    for (index, character) in digits.chars().enumerate() {
        if !character.is_ascii_digit() {
            let position = start + index + 1;
            return Err(ParseError::InvalidCharacter {
                character,
                position,
            });
        }
    }
    // Stop growing once the number is past every limit, so it cannot
    // overflow however many digits there are.
    let limit = i64::from(i32::MAX) + 2;
    let magnitude = digits.bytes().fold(0_i64, |total, digit| {
        (total * 10 + i64::from(digit - b'0')).min(limit)
    });
    let value = if negative { -magnitude } else { magnitude };
    i32::try_from(value).map_err(|_| {
        if negative {
            ParseError::TooSmall
        } else {
            ParseError::TooLarge
        }
    })
}

fn main() {
    for line in io::stdin().lock().lines() {
        let line = line.expect("stdin should be readable");
        match parse_int(&line) {
            Ok(value) => println!("\"{line}\" -> {value}"),
            Err(error) => println!("\"{line}\" -> error: {error}"),
        }
    }
}
