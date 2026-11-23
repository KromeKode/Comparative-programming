//! Copies stdin to stdout, with each date in it written as YYYY-MM-DD.
use std::io::{self, Read, Write};

use regex::{Captures, Regex};

const MONTHS: [&str; 12] = [
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December",
];

fn days_in(month: u32, year: u32) -> u32 {
    let leap = year.is_multiple_of(4) && (!year.is_multiple_of(100) || year.is_multiple_of(400));
    match month {
        2 if leap => 29,
        2 => 28,
        4 | 6 | 9 | 11 => 30,
        _ => 31,
    }
}

/// The date as YYYY-MM-DD, or the text as it was if it isn't a real date.
fn rewrite(caps: &Captures) -> String {
    // Each number is at most four ASCII digits, so parse can't fail.
    let number = |i| caps.get(i).map(|m| m.as_str().parse::<u32>().unwrap());
    let month_of = |i| {
        let name = caps.get(i)?.as_str();
        Some(MONTHS.iter().position(|m| m.starts_with(name))? as u32 + 1)
    };
    // Only one of the three forms matched, so the groups of the other two are None.
    let (day, month, year) = if let Some(day) = number(1) {
        (day, number(2).unwrap(), number(3).unwrap())
    } else if let Some(month) = month_of(4) {
        (number(5).unwrap(), month, number(6).unwrap())
    } else {
        (number(7).unwrap(), month_of(8).unwrap(), number(9).unwrap())
    };
    if (1..=12).contains(&month) && (1..=days_in(month, year)).contains(&day) {
        format!("{year:04}-{month:02}-{day:02}")
    } else {
        caps[0].to_string() // not a real date, so left as it was
    }
}

fn main() -> io::Result<()> {
    let month = MONTHS
        .map(|name| {
            if name.len() > 3 {
                format!("{}(?:{})?", &name[..3], &name[3..])
            } else {
                name.to_string()
            }
        })
        .join("|");
    let (d, m, y) = ("([0-9]{1,2})", "([0-9]{1,2})", "([0-9]{4})");
    // In the regex crate, \b is Unicode's, and takes "é" for a letter. (?-u:\b) is ASCII's,
    // which looks at ASCII letters, digits, and _ alone. [0-9], not \d, which takes "٣".
    let date = Regex::new(&format!(
        r"(?-u:\b)(?:{d}/{m}/{y}|({month}) {d}, {y}|{d} ({month}) {y})(?-u:\b)"
    ))
    .unwrap();
    let mut text = String::new();
    io::stdin().read_to_string(&mut text)?;
    io::stdout().write_all(date.replace_all(&text, rewrite).as_bytes())
}
