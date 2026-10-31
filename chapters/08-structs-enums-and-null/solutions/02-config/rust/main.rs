use std::io::{self, BufRead};

// None means "not set", which is different from every real value, the default included.
#[derive(Default)]
struct Config {
    host: Option<String>,
    port: Option<u32>,
    timeout: Option<u32>,
}

// The number, if text is digits only and from 1 to high.
fn parse_number(text: &str, high: u32) -> Option<u32> {
    if text.is_empty() || !text.bytes().all(|b| b.is_ascii_digit()) {
        return None;
    }
    // parse fails for numbers too big for a u32; filter drops the ones out of range.
    text.parse().ok().filter(|n| (1..=high).contains(n))
}

// Sets setting from value, or reports a bad value and leaves it alone.
fn set_number(setting: &mut Option<u32>, key: &str, value: &str, high: u32, number: usize) {
    if value.is_empty() {
        *setting = None;
    } else if let Some(parsed) = parse_number(value, high) {
        *setting = Some(parsed);
    } else {
        println!("line {number}: bad {key} \"{value}\"");
    }
}

fn main() {
    let mut config = Config::default();
    for (index, line) in io::stdin().lock().lines().enumerate() {
        let line = line.expect("failed to read stdin");
        let line = line.trim();
        let number = index + 1;
        if line.is_empty() || line.starts_with('#') {
            continue;
        }
        let Some((key, value)) = line.split_once('=') else {
            println!("line {number}: expected key = value");
            continue;
        };
        let (key, value) = (key.trim(), value.trim());
        match key {
            "host" => config.host = Some(value.to_string()).filter(|v| !v.is_empty()),
            "port" => set_number(&mut config.port, key, value, 65535, number),
            "timeout" => set_number(&mut config.timeout, key, value, 86400, number),
            _ => println!("line {number}: unknown key \"{key}\""),
        }
    }

    match config.host {
        Some(host) => println!("host: {host}"),
        None => println!("host: localhost (default)"),
    }
    match config.port {
        Some(port) => println!("port: {port}"),
        None => println!("port: 8080 (default)"),
    }
    match config.timeout {
        Some(timeout) => println!("timeout: {timeout}s"),
        None => println!("timeout: none"),
    }
}
