//! Versions and ranges as npm writes them: 1.2.3-beta.1, ^1.2, ~1.2.3, >=1 <2, 1.x || 3.
//!
//! Only the items marked `pub` can be used from main.rs: Version, its fields and
//! methods, compare, Range, and Range's methods. Everything else is private to this
//! module.

use std::cmp::Ordering;
use std::fmt;

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct Version {
    pub major: u64,
    pub minor: u64,
    pub patch: u64,
    /// The pre-release's identifiers, if it has one.
    pub pre: Vec<String>,
}

impl Version {
    /// The version that text is, or None if it isn't one.
    pub fn parse(text: &str) -> Option<Version> {
        // The core is everything before the first '-', and the pre-release after it.
        let (core, pre) = match text.split_once('-') {
            Some((core, pre)) => (core, Some(pre)),
            None => (text, None),
        };
        let parts: Vec<&str> = core.split('.').collect();
        let [major, minor, patch] = parts[..] else {
            return None;
        };
        Some(Version {
            major: parse_number(major)?,
            minor: parse_number(minor)?,
            patch: parse_number(patch)?,
            pre: match pre {
                Some(pre) => parse_pre(pre)?,
                None => Vec::new(),
            },
        })
    }

    fn core(&self) -> (u64, u64, u64) {
        (self.major, self.minor, self.patch)
    }
}

impl fmt::Display for Version {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}.{}.{}", self.major, self.minor, self.patch)?;
        if !self.pre.is_empty() {
            write!(f, "-{}", self.pre.join("."))?;
        }
        Ok(())
    }
}

fn all_digits(text: &str) -> bool {
    text.bytes().all(|b| b.is_ascii_digit())
}

/// A number in a version: 0, or up to 15 digits that don't start with 0.
fn parse_number(text: &str) -> Option<u64> {
    let ok = !text.is_empty() && text.len() <= 15 && all_digits(text);
    if !ok || (text.len() > 1 && text.starts_with('0')) {
        return None;
    }
    text.parse().ok()
}

/// A pre-release is identifiers separated by dots. Each is digits without a 0 in
/// front, or letters, digits, and hyphens with at least one that isn't a digit.
fn parse_pre(text: &str) -> Option<Vec<String>> {
    let mut identifiers = Vec::new();
    for identifier in text.split('.') {
        let ok = !identifier.is_empty()
            && identifier
                .bytes()
                .all(|b| b.is_ascii_alphanumeric() || b == b'-');
        let zero_in_front =
            identifier.len() > 1 && identifier.starts_with('0') && all_digits(identifier);
        if !ok || zero_in_front {
            return None;
        }
        identifiers.push(identifier.to_string());
    }
    Some(identifiers)
}

/// Numbers compare as numbers, and come before other identifiers, which compare
/// character by character. Numbers have no 0 in front, so a longer one is larger.
fn compare_identifiers(a: &str, b: &str) -> Ordering {
    match (all_digits(a), all_digits(b)) {
        (true, true) => a.len().cmp(&b.len()).then_with(|| a.cmp(b)),
        (true, false) => Ordering::Less,
        (false, true) => Ordering::Greater,
        (false, false) => a.cmp(b),
    }
}

/// Less if a comes before b, Equal if they are the same, and Greater if after.
pub fn compare(a: &Version, b: &Version) -> Ordering {
    a.core().cmp(&b.core()).then_with(|| {
        match (a.pre.is_empty(), b.pre.is_empty()) {
            // A version without a pre-release comes after the same version with one.
            (true, true) => Ordering::Equal,
            (true, false) => Ordering::Greater,
            (false, true) => Ordering::Less,
            (false, false) => a
                .pre
                .iter()
                .zip(&b.pre)
                .map(|(x, y)| compare_identifiers(x, y))
                .find(|order| order.is_ne())
                .unwrap_or_else(|| a.pre.len().cmp(&b.pre.len())),
        }
    })
}

#[derive(Clone, Copy)]
enum Op {
    Eq,
    Lt,
    Le,
    Gt,
    Ge,
}

/// A simple comparator: an operator and a full version.
type Simple = (Op, Version);

/// A range: sets of simple comparators joined by ||. A version is in the range if
/// it passes every comparator of some set. The field is private, so the only way
/// to make a Range is Range::parse.
pub struct Range {
    sets: Vec<Vec<Simple>>,
}

fn version(major: u64, minor: u64, patch: u64, pre: &[&str]) -> Version {
    let pre = pre.iter().map(|s| s.to_string()).collect();
    Version {
        major,
        minor,
        patch,
        pre,
    }
}

/// The simple comparators that a comparator stands for, or None if it isn't one.
fn expand(token: &str) -> Option<Vec<Simple>> {
    let (kind, rest) = ["<=", ">=", "<", ">", "=", "^", "~"]
        .iter()
        .find_map(|op| token.strip_prefix(op).map(|rest| (*op, rest)))
        .unwrap_or(("", token));
    let (core, pre) = match rest.split_once('-') {
        Some((core, pre)) => (core, Some(pre)),
        None => (rest, None),
    };
    let parts: Vec<&str> = core.split('.').collect();
    if parts.len() > 3 {
        return None;
    }
    // known counts the numbers, which must come before any wildcards.
    let mut nums = [0; 3];
    let mut known = 0;
    for (i, part) in parts.iter().enumerate() {
        if matches!(*part, "x" | "X" | "*") {
            continue;
        }
        if known != i {
            return None;
        }
        nums[known] = parse_number(part)?;
        known += 1;
    }
    let mut low = version(nums[0], nums[1], nums[2], &[]);
    if let Some(pre) = pre {
        // A pre-release can follow only three numbers.
        if known < 3 {
            return None;
        }
        low.pre = parse_pre(pre)?;
    }
    // The first version after all those whose parts up to level match: the -0
    // pre-release of the next major, minor, or patch.
    let after = |level: usize| match level {
        0 => version(nums[0] + 1, 0, 0, &["0"]),
        1 => version(nums[0], nums[1] + 1, 0, &["0"]),
        _ => version(nums[0], nums[1], nums[2] + 1, &["0"]),
    };
    if known == 0 {
        // Any version: nothing is outside it, and nothing is above or below it.
        return Some(match kind {
            "<" | ">" => vec![(Op::Lt, version(0, 0, 0, &["0"]))],
            _ => vec![],
        });
    }
    let next = after(known - 1);
    Some(match (kind, known) {
        ("" | "=", 3) => vec![(Op::Eq, low)],
        ("" | "=", _) => vec![(Op::Ge, low), (Op::Lt, next)],
        (">", 3) => vec![(Op::Gt, low)],
        (">", _) => vec![(Op::Ge, version(next.major, next.minor, next.patch, &[]))],
        (">=", _) => vec![(Op::Ge, low)],
        ("<", 3) => vec![(Op::Lt, low)],
        ("<", _) => vec![(Op::Lt, version(low.major, low.minor, low.patch, &["0"]))],
        ("<=", 3) => vec![(Op::Le, low)],
        ("<=", _) => vec![(Op::Lt, next)],
        ("~", _) => vec![
            (Op::Ge, low),
            (Op::Lt, after(if known == 1 { 0 } else { 1 })),
        ],
        _ => {
            // ^ allows changes that don't change the first part that isn't 0.
            let first = (0..known).find(|&i| nums[i] != 0).unwrap_or(known - 1);
            vec![(Op::Ge, low), (Op::Lt, after(first))]
        }
    })
}

fn holds(version: &Version, (op, bound): &Simple) -> bool {
    let order = compare(version, bound);
    match op {
        Op::Eq => order.is_eq(),
        Op::Lt => order.is_lt(),
        Op::Le => order.is_le(),
        Op::Gt => order.is_gt(),
        Op::Ge => order.is_ge(),
    }
}

impl Range {
    /// The range that text is, or an Err saying why it isn't one.
    pub fn parse(text: &str) -> Result<Range, String> {
        let mut sets = Vec::new();
        for side in text.split("||") {
            let tokens: Vec<&str> = side.split_whitespace().collect();
            if tokens.is_empty() {
                return Err("each side of || needs a comparator".to_string());
            }
            let mut simples = Vec::new();
            for token in tokens {
                simples.extend(expand(token).ok_or(format!("{token} isn't a comparator"))?);
            }
            sets.push(simples);
        }
        Ok(Range { sets })
    }

    pub fn allows(&self, version: &Version) -> bool {
        self.sets.iter().any(|simples| {
            // A pre-release is in only if a comparator of the set names a
            // pre-release of the same major, minor, and patch.
            simples.iter().all(|simple| holds(version, simple))
                && (version.pre.is_empty()
                    || simples
                        .iter()
                        .any(|(_, bound)| !bound.pre.is_empty() && bound.core() == version.core()))
        })
    }

    /// The highest of versions that the range allows, or None if it allows none.
    pub fn highest<'a>(&self, versions: &'a [Version]) -> Option<&'a Version> {
        versions
            .iter()
            .filter(|version| self.allows(version))
            .max_by(|a, b| compare(a, b))
    }
}
