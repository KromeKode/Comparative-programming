use std::collections::HashMap;
use std::fmt;
use std::io::{self, Read};

struct Config {
    name: String,
    port: u32,
    debug: bool,
    timeout: u32,
}

// What was wrong with a value.
enum ValueProblem {
    Empty,
    NotWhole(String),
    OutOfRange { text: String, low: u32, high: u32 },
    NotBool(String),
}

impl fmt::Display for ValueProblem {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            ValueProblem::Empty => write!(f, "must not be empty"),
            ValueProblem::NotWhole(text) => write!(f, "\"{text}\" is not a whole number"),
            ValueProblem::OutOfRange { text, low, high } => {
                write!(f, "{text} is not between {low} and {high}")
            }
            ValueProblem::NotBool(text) => write!(f, "\"{text}\" is not true or false"),
        }
    }
}

enum ConfigError {
    Syntax {
        line: usize,
    },
    UnknownKey {
        line: usize,
        key: String,
    },
    DuplicateKey {
        line: usize,
        key: String,
        first: usize,
    },
    BadValue {
        line: usize,
        key: String,
        problem: ValueProblem,
    },
    MissingKey {
        key: String,
    },
}

const KINDS: [&str; 5] = [
    "syntax",
    "unknown key",
    "duplicate key",
    "bad value",
    "missing key",
];

impl ConfigError {
    // The position of this error's kind in KINDS.
    fn kind(&self) -> usize {
        match self {
            ConfigError::Syntax { .. } => 0,
            ConfigError::UnknownKey { .. } => 1,
            ConfigError::DuplicateKey { .. } => 2,
            ConfigError::BadValue { .. } => 3,
            ConfigError::MissingKey { .. } => 4,
        }
    }
}

impl fmt::Display for ConfigError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            ConfigError::Syntax { line } => write!(f, "line {line}: expected \"key = value\""),
            ConfigError::UnknownKey { line, key } => {
                write!(f, "line {line}: unknown key \"{key}\"")
            }
            ConfigError::DuplicateKey { line, key, first } => {
                write!(f, "line {line}: \"{key}\" is already set on line {first}")
            }
            ConfigError::BadValue { line, key, problem } => {
                write!(f, "line {line}: {key}: {problem}")
            }
            ConfigError::MissingKey { key } => write!(f, "missing key \"{key}\""),
        }
    }
}

fn parse_text(text: &str) -> Result<String, ValueProblem> {
    if text.is_empty() {
        return Err(ValueProblem::Empty);
    }
    Ok(text.to_string())
}

fn parse_whole(text: &str, low: u32, high: u32) -> Result<u32, ValueProblem> {
    if text.is_empty() || !text.bytes().all(|b| b.is_ascii_digit()) {
        return Err(ValueProblem::NotWhole(text.to_string()));
    }
    // Stop growing past the limit, so a long number cannot overflow.
    let value = text.bytes().fold(0, |total, b| {
        (total * 10 + u32::from(b - b'0')).min(high + 1)
    });
    if !(low..=high).contains(&value) {
        let text = text.to_string();
        return Err(ValueProblem::OutOfRange { text, low, high });
    }
    Ok(value)
}

fn parse_bool(text: &str) -> Result<bool, ValueProblem> {
    match text {
        "true" => Ok(true),
        "false" => Ok(false),
        _ => Err(ValueProblem::NotBool(text.to_string())),
    }
}

fn load(lines: &[&str]) -> Result<Config, Vec<ConfigError>> {
    let mut name = None;
    let mut port = None;
    let mut debug = false;
    let mut timeout = 30;
    let mut first_line: HashMap<&str, usize> = HashMap::new();
    let mut errors = Vec::new();
    for (index, text_line) in lines.iter().enumerate() {
        let line = index + 1;
        let text_line = text_line.trim_matches(' ');
        if text_line.is_empty() || text_line.starts_with('#') {
            continue;
        }
        let Some((key, text)) = text_line.split_once('=') else {
            errors.push(ConfigError::Syntax { line });
            continue;
        };
        let (key, text) = (key.trim_matches(' '), text.trim_matches(' '));
        if key.is_empty() {
            errors.push(ConfigError::Syntax { line });
            continue;
        }
        if !["name", "port", "debug", "timeout"].contains(&key) {
            let key = key.to_string();
            errors.push(ConfigError::UnknownKey { line, key });
            continue;
        }
        if let Some(&first) = first_line.get(key) {
            let key = key.to_string();
            errors.push(ConfigError::DuplicateKey { line, key, first });
            continue;
        }
        first_line.insert(key, line);
        // Each arm parses the value and, if that worked, stores it.
        let parsed = match key {
            "name" => parse_text(text).map(|value| name = Some(value)),
            "port" => parse_whole(text, 1, 65535).map(|value| port = Some(value)),
            "debug" => parse_bool(text).map(|value| debug = value),
            _ => parse_whole(text, 1, 3600).map(|value| timeout = value),
        };
        if let Err(problem) = parsed {
            let key = key.to_string();
            errors.push(ConfigError::BadValue { line, key, problem });
        }
    }
    for key in ["name", "port"] {
        if !first_line.contains_key(key) {
            let key = key.to_string();
            errors.push(ConfigError::MissingKey { key });
        }
    }
    match (name, port) {
        (Some(name), Some(port)) if errors.is_empty() => Ok(Config {
            name,
            port,
            debug,
            timeout,
        }),
        _ => Err(errors),
    }
}

fn main() {
    let mut input = String::new();
    let mut stdin = io::stdin();
    stdin.read_to_string(&mut input).expect("stdin is readable");
    let mut files: Vec<(&str, Vec<&str>)> = Vec::new();
    for line in input.lines() {
        if let Some(name) = line.strip_prefix("== ") {
            files.push((name, Vec::new()));
        } else if let Some((_, lines)) = files.last_mut() {
            lines.push(line);
        }
    }
    let mut counts = [0; KINDS.len()];
    let mut loaded = 0;
    for (name, lines) in &files {
        match load(lines) {
            Ok(config) => {
                loaded += 1;
                println!("{name}: ok");
                println!("  name = {}", config.name);
                println!("  port = {}", config.port);
                println!("  debug = {}", config.debug);
                println!("  timeout = {}", config.timeout);
            }
            Err(errors) => {
                let plural = if errors.len() == 1 { "" } else { "s" };
                println!("{name}: {} error{plural}", errors.len());
                for error in &errors {
                    println!("  {error}");
                    counts[error.kind()] += 1;
                }
            }
        }
    }
    println!("files loaded: {loaded} of {}", files.len());
    let summary: Vec<String> = KINDS
        .iter()
        .zip(counts)
        .map(|(kind, count)| format!("{kind}: {count}"))
        .collect();
    println!("{}", summary.join(", "));
}
