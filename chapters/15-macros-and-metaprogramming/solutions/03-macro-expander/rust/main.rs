use std::collections::{HashMap, HashSet};
use std::io::{self, Read};

struct Macro {
    params: Option<Vec<String>>, // None for a macro that takes no parentheses
    body: String,
}

fn is_word_byte(b: u8) -> bool {
    b.is_ascii_alphanumeric() || b == b'_'
}

fn word_end(text: &str, start: usize) -> usize {
    start
        + text[start..]
            .bytes()
            .take_while(|&b| is_word_byte(b))
            .count()
}

fn spaces_end(text: &str, start: usize) -> usize {
    start + text[start..].bytes().take_while(|&b| b == b' ').count()
}

// The index just past the character at text[start]. A character can take
// more than one byte, and slicing a str in the middle of one panics.
fn char_end(text: &str, start: usize) -> usize {
    start + text[start..].chars().next().map_or(1, char::len_utf8)
}

// The index just past the string that starts at text[start], a quote.
fn string_end(text: &str, start: usize) -> usize {
    text[start + 1..]
        .find('"')
        .map_or(text.len(), |close| start + close + 2)
}

fn is_name(word: &str) -> bool {
    !word.is_empty() && !word.as_bytes()[0].is_ascii_digit() && word_end(word, 0) == word.len()
}

// Splits the arguments of a call at its top-level commas, and trims them.
// Returns them with the index just past the ")".
fn split_args<'t>(text: &'t str, open: usize, name: &str) -> Result<(Vec<&'t str>, usize), String> {
    let bytes = text.as_bytes();
    let mut args = Vec::new();
    let mut depth = 0;
    let (mut start, mut i) = (open + 1, open + 1);
    while i < bytes.len() {
        match bytes[i] {
            b'"' => {
                i = string_end(text, i);
                continue;
            }
            b'(' => depth += 1,
            b')' if depth > 0 => depth -= 1,
            b')' => {
                args.push(text[start..i].trim());
                return Ok((args, i + 1));
            }
            b',' if depth == 0 => {
                args.push(text[start..i].trim());
                start = i + 1;
            }
            _ => {}
        }
        i += 1;
    }
    Err(format!("unterminated call to {name}"))
}

#[derive(Default)]
struct Expander {
    macros: HashMap<String, Macro>,
}

impl Expander {
    // Expands every macro in text, except those in disabled. The names in
    // disabled are borrowed from the keys of self.macros, so they live as
    // long as the borrow of self does, which is 'm.
    fn expand<'m>(&'m self, text: &str, disabled: &HashSet<&'m str>) -> Result<String, String> {
        let bytes = text.as_bytes();
        let mut out = String::new();
        let mut i = 0;
        while i < text.len() {
            let mut end = char_end(text, i);
            if bytes[i] == b'"' {
                end = string_end(text, i);
            } else if is_word_byte(bytes[i]) {
                end = word_end(text, i);
                if let Some((name, mac)) = self.macros.get_key_value(&text[i..end])
                    && !disabled.contains(name.as_str())
                {
                    // Inside its own expansion, a macro's name is left alone.
                    let mut inner = disabled.clone();
                    inner.insert(name);
                    let Some(params) = &mac.params else {
                        out += &self.expand(&mac.body, &inner)?;
                        i = end;
                        continue;
                    };
                    let paren = spaces_end(text, end);
                    if text[paren..].starts_with('(') {
                        let (mut args, after) = split_args(text, paren, name)?;
                        if params.is_empty() && args == [""] {
                            args.clear();
                        }
                        if args.len() != params.len() {
                            let count = params.len();
                            let noun = if count == 1 { "argument" } else { "arguments" };
                            return Err(format!("{name} takes {count} {noun}, got {}", args.len()));
                        }
                        let body = self.substitute(params, &mac.body, &args, disabled)?;
                        out += &self.expand(&body, &inner)?;
                        i = after;
                        continue;
                    }
                }
            }
            out += &text[i..end];
            i = end;
        }
        Ok(out)
    }

    // The macro's body, with each parameter replaced by its argument.
    fn substitute<'m>(
        &'m self,
        params: &[String],
        body: &str,
        args: &[&str],
        disabled: &HashSet<&'m str>,
    ) -> Result<String, String> {
        let raw: HashMap<&str, &str> = params
            .iter()
            .map(String::as_str)
            .zip(args.iter().copied())
            .collect();
        let mut expanded: HashMap<&str, String> = HashMap::new(); // only the arguments used
        let bytes = body.as_bytes();
        let mut out = String::new();
        let mut i = 0;
        while i < body.len() {
            let mut end = char_end(body, i);
            if bytes[i] == b'"' {
                end = string_end(body, i);
            } else if bytes[i] == b'#' {
                // # and a parameter make a string of the argument as written.
                let start = spaces_end(body, i + 1);
                let stop = word_end(body, start);
                if let Some(arg) = raw.get(&body[start..stop]) {
                    out += &format!("\"{arg}\"");
                    i = stop;
                    continue;
                }
            } else if is_word_byte(bytes[i]) {
                end = word_end(body, i);
                let word = &body[i..end];
                if let Some(arg) = raw.get(word) {
                    if !expanded.contains_key(word) {
                        expanded.insert(word, self.expand(arg, disabled)?);
                    }
                    out += &expanded[word];
                    i = end;
                    continue;
                }
            }
            out += &body[i..end];
            i = end;
        }
        Ok(out)
    }

    fn define(&mut self, rest: &str) -> Result<(), String> {
        let bad = || "bad #define".to_string();
        let end = word_end(rest, 0);
        let name = &rest[..end];
        if !is_name(name) {
            return Err(bad());
        }
        let mac = if let Some(after_open) = rest[end..].strip_prefix('(') {
            let (inside, body) = after_open.split_once(')').ok_or_else(bad)?;
            let params: Vec<String> = if inside.trim().is_empty() {
                Vec::new()
            } else {
                inside
                    .split(',')
                    .map(|param| param.trim().to_string())
                    .collect()
            };
            let unique: HashSet<&String> = params.iter().collect();
            if !params.iter().all(|param| is_name(param)) || unique.len() != params.len() {
                return Err(bad());
            }
            Macro {
                params: Some(params),
                body: body.trim().to_string(),
            }
        } else {
            Macro {
                params: None,
                body: rest[end..].trim().to_string(),
            }
        };
        self.macros.insert(name.to_string(), mac);
        Ok(())
    }

    // Returns the line to print, or None for a directive.
    fn run(&mut self, line: &str) -> Result<Option<String>, String> {
        let whole = line.trim();
        let (directive, rest) = whole.split_once(' ').unwrap_or((whole, ""));
        let rest = rest.trim();
        match directive {
            "#define" => self.define(rest).map(|()| None),
            "#undef" if is_name(rest) => {
                self.macros.remove(rest);
                Ok(None)
            }
            "#undef" => Err("bad #undef".to_string()),
            _ => self.expand(line, &HashSet::new()).map(Some),
        }
    }
}

fn main() {
    let mut input = String::new();
    io::stdin()
        .read_to_string(&mut input)
        .expect("can't read stdin");
    let mut expander = Expander::default();
    for line in input.lines() {
        match expander.run(line) {
            Ok(Some(text)) => println!("{text}"),
            Ok(None) => {}
            Err(error) => println!("error: {error}"),
        }
    }
}
