use std::fmt::Display;
use std::io::{self, BufRead};

/// What eval needs from a number type. parse has no self: it makes a new
/// value, and eval calls it as N::parse without having an N yet.
trait Number: Sized + Display {
    fn parse(text: &str) -> Option<Self>;
    fn add(&self, other: &Self) -> Self;
    fn sub(&self, other: &Self) -> Self;
    fn mul(&self, other: &Self) -> Self;
    fn div(&self, other: &Self) -> Option<Self>; // None when dividing by zero
}

/// A whole number: an optional minus sign and at least one digit.
fn parse_whole(text: &str) -> Option<i64> {
    let digits = text.strip_prefix('-').unwrap_or(text);
    if digits.is_empty() || !digits.bytes().all(|b| b.is_ascii_digit()) {
        return None;
    }
    text.parse().ok()
}

struct Int(i64);

impl Number for Int {
    fn parse(text: &str) -> Option<Self> {
        parse_whole(text).map(Int)
    }

    fn add(&self, other: &Self) -> Self {
        Int(self.0 + other.0)
    }

    fn sub(&self, other: &Self) -> Self {
        Int(self.0 - other.0)
    }

    fn mul(&self, other: &Self) -> Self {
        Int(self.0 * other.0)
    }

    fn div(&self, other: &Self) -> Option<Self> {
        // checked_div is None for a zero divisor. Rust's / rounds toward zero.
        self.0.checked_div(other.0).map(Int)
    }
}

impl Display for Int {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "{}", self.0)
    }
}

/// Always in lowest terms, with a positive bottom.
struct Frac {
    top: i64,
    bottom: i64,
}

fn gcd(a: i64, b: i64) -> i64 {
    if b == 0 { a.abs() } else { gcd(b, a % b) }
}

impl Frac {
    fn new(top: i64, bottom: i64) -> Frac {
        let sign = if bottom < 0 { -1 } else { 1 };
        let divisor = gcd(top, bottom);
        Frac {
            top: sign * top / divisor,
            bottom: sign * bottom / divisor,
        }
    }
}

impl Number for Frac {
    fn parse(text: &str) -> Option<Self> {
        let (top, bottom) = text.split_once('/').unwrap_or((text, "1"));
        let (top, bottom) = (parse_whole(top)?, parse_whole(bottom)?);
        // "1/-2" and "1/0" are not fractions: the bottom must be above zero.
        (bottom > 0).then(|| Frac::new(top, bottom))
    }

    fn add(&self, other: &Self) -> Self {
        Frac::new(
            self.top * other.bottom + other.top * self.bottom,
            self.bottom * other.bottom,
        )
    }

    fn sub(&self, other: &Self) -> Self {
        Frac::new(
            self.top * other.bottom - other.top * self.bottom,
            self.bottom * other.bottom,
        )
    }

    fn mul(&self, other: &Self) -> Self {
        Frac::new(self.top * other.top, self.bottom * other.bottom)
    }

    fn div(&self, other: &Self) -> Option<Self> {
        (other.top != 0).then(|| Frac::new(self.top * other.bottom, self.bottom * other.top))
    }
}

impl Display for Frac {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        if self.bottom == 1 {
            write!(f, "{}", self.top)
        } else {
            write!(f, "{}/{}", self.top, self.bottom)
        }
    }
}

/// Always 0 to 6.
struct Mod7(i64);

impl Number for Mod7 {
    fn parse(text: &str) -> Option<Self> {
        // rem_euclid is never negative, unlike %: -1 % 7 is -1.
        parse_whole(text).map(|n| Mod7(n.rem_euclid(7)))
    }

    fn add(&self, other: &Self) -> Self {
        Mod7((self.0 + other.0) % 7)
    }

    fn sub(&self, other: &Self) -> Self {
        Mod7((self.0 - other.0).rem_euclid(7))
    }

    fn mul(&self, other: &Self) -> Self {
        Mod7(self.0 * other.0 % 7)
    }

    fn div(&self, other: &Self) -> Option<Self> {
        // x to the power 5 is the inverse of x when 7 is prime: x * x^5 = x^6 = 1.
        (other.0 != 0).then(|| self.mul(&Mod7(other.0.pow(5) % 7)))
    }
}

impl Display for Mod7 {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "{}", self.0)
    }
}

/// Runs the tokens on a stack of N. The same code works for every number type.
fn eval<N: Number>(tokens: &[&str]) -> Result<N, String> {
    let mut stack: Vec<N> = Vec::new();
    for &token in tokens {
        if let "+" | "-" | "*" | "/" = token {
            let (Some(right), Some(left)) = (stack.pop(), stack.pop()) else {
                return Err("not enough numbers".to_string());
            };
            let result = match token {
                "+" => left.add(&right),
                "-" => left.sub(&right),
                "*" => left.mul(&right),
                _ => left.div(&right).ok_or("division by zero")?,
            };
            stack.push(result);
        } else {
            stack.push(N::parse(token).ok_or_else(|| format!("bad token \"{token}\""))?);
        }
    }
    match <[N; 1]>::try_from(stack) {
        Ok([result]) => Ok(result),
        Err(stack) => Err(format!("expected one result, found {}", stack.len())),
    }
}

fn main() {
    for line in io::stdin().lock().lines() {
        let line = line.expect("read stdin");
        let words: Vec<&str> = line.split_whitespace().collect();
        let Some((&kind, tokens)) = words.split_first() else {
            continue;
        };
        let result = match kind {
            "int" => eval::<Int>(tokens).map(|n| n.to_string()),
            "frac" => eval::<Frac>(tokens).map(|n| n.to_string()),
            "mod7" => eval::<Mod7>(tokens).map(|n| n.to_string()),
            _ => Err(format!("unknown kind \"{kind}\"")),
        };
        match result {
            Ok(text) => println!("{text}"),
            Err(message) => println!("error: {message}"),
        }
    }
}
