//! Durations such as 1h30m: `parse_duration` reads one, and `format_duration` writes one.

use std::fmt;

/// The units, largest first, which is the order they come in, and the seconds in each.
const UNITS: [(char, u64); 4] = [('d', 86400), ('h', 3600), ('m', 60), ('s', 1)];

/// The error for text that isn't a duration.
#[derive(Debug, PartialEq, Eq)]
pub struct ParseDurationError(String);

impl fmt::Display for ParseDurationError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "not a duration: {}", self.0)
    }
}

impl std::error::Error for ParseDurationError {}

/// The number of seconds that `text` says, such as 5400 for "1h30m".
pub fn parse_duration(text: &str) -> Result<u64, ParseDurationError> {
    let error = |why: &str| Err(ParseDurationError(format!("{text:?} {why}")));
    if text.is_empty() {
        return error("is empty");
    }
    let mut seconds: u64 = 0;
    let mut next_unit = 0; // each part's unit comes after the one before it in UNITS
    let mut rest = text;
    while !rest.is_empty() {
        let digits = rest
            .find(|c: char| !c.is_ascii_digit())
            .unwrap_or(rest.len());
        if digits == 0 {
            return error("has a part with no number");
        }
        if digits == rest.len() {
            return error("ends with a number that has no unit");
        }
        let Ok(number) = rest[..digits].parse::<u64>() else {
            return error("has a number too big to count");
        };
        let letter = rest[digits..].chars().next().unwrap_or_default();
        let Some(unit) = UNITS.iter().position(|&(name, _)| name == letter) else {
            return error("has something that isn't a unit");
        };
        if unit < next_unit {
            return error("has its units out of order, or one twice");
        }
        let Some(total) = number
            .checked_mul(UNITS[unit].1)
            .and_then(|part| seconds.checked_add(part))
        else {
            return error("is too long to count");
        };
        seconds = total;
        next_unit = unit + 1;
        rest = &rest[digits + letter.len_utf8()..];
    }
    Ok(seconds)
}

/// `seconds` as a duration, such as "1h30m" for 5400, leaving out the units that are 0.
pub fn format_duration(mut seconds: u64) -> String {
    if seconds == 0 {
        return "0s".to_string();
    }
    let mut text = String::new();
    for (name, size) in UNITS {
        let amount = seconds / size;
        seconds %= size;
        if amount > 0 {
            text.push_str(&format!("{amount}{name}"));
        }
    }
    text
}

#[cfg(test)]
mod tests;
