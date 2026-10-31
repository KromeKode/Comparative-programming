use std::io::{self, BufRead};

// Every plugin has a name and changes a list of words. describe has a default
// body, so only plugins with settings need to write it.
trait Plugin {
    fn name(&self) -> &'static str;
    fn apply(&self, words: Vec<String>) -> Vec<String>;

    fn describe(&self) -> String {
        self.name().to_string()
    }
}

#[derive(Default)]
struct Upper;

impl Plugin for Upper {
    fn name(&self) -> &'static str {
        "upper"
    }

    fn apply(&self, words: Vec<String>) -> Vec<String> {
        words.iter().map(|word| word.to_ascii_uppercase()).collect()
    }
}

#[derive(Default)]
struct Lower;

impl Plugin for Lower {
    fn name(&self) -> &'static str {
        "lower"
    }

    fn apply(&self, words: Vec<String>) -> Vec<String> {
        words.iter().map(|word| word.to_ascii_lowercase()).collect()
    }
}

#[derive(Default)]
struct Reverse;

impl Plugin for Reverse {
    fn name(&self) -> &'static str {
        "reverse"
    }

    fn apply(&self, mut words: Vec<String>) -> Vec<String> {
        words.reverse();
        words
    }
}

struct Censor {
    word: String,
}

impl Plugin for Censor {
    fn name(&self) -> &'static str {
        "censor"
    }

    fn apply(&self, words: Vec<String>) -> Vec<String> {
        let hide = |word: String| {
            if word.eq_ignore_ascii_case(&self.word) {
                "*".repeat(word.len())
            } else {
                word
            }
        };
        words.into_iter().map(hide).collect()
    }

    fn describe(&self) -> String {
        format!("censor({})", self.word)
    }
}

type Make = fn(&[&str]) -> Result<Box<dyn Plugin>, String>;

struct Entry {
    name: &'static str,
    usage: &'static str,
    help: &'static str,
    make: Make,
}

// Maps a plugin's name to a way to make one. The rest of the program finds
// plugins only through here.
#[derive(Default)]
struct Registry {
    entries: Vec<Entry>,
}

impl Registry {
    fn register(&mut self, entry: Entry) {
        self.entries.push(entry);
    }

    fn create(&self, name: &str, args: &[&str]) -> Result<Box<dyn Plugin>, String> {
        let entry = self
            .entries
            .iter()
            .find(|entry| entry.name == name)
            .ok_or(format!("no plugin named {name}"))?;
        (entry.make)(args)
    }
}

// One function for every plugin that takes no arguments. Each use, such as
// no_arguments::<Upper>, is a separate function made for that type.
fn no_arguments<P: Plugin + Default + 'static>(args: &[&str]) -> Result<Box<dyn Plugin>, String> {
    let plugin = P::default();
    if !args.is_empty() {
        return Err(format!("{} takes no arguments", plugin.name()));
    }
    Ok(Box::new(plugin))
}

fn make_censor(args: &[&str]) -> Result<Box<dyn Plugin>, String> {
    match args {
        [word] => Ok(Box::new(Censor {
            word: word.to_string(),
        })),
        _ => Err("censor needs exactly one word".to_string()),
    }
}

fn make_registry() -> Registry {
    let mut registry = Registry::default();
    registry.register(Entry {
        name: "upper",
        usage: "upper",
        help: "make every letter uppercase",
        make: no_arguments::<Upper>,
    });
    registry.register(Entry {
        name: "lower",
        usage: "lower",
        help: "make every letter lowercase",
        make: no_arguments::<Lower>,
    });
    registry.register(Entry {
        name: "reverse",
        usage: "reverse",
        help: "reverse the order of the words",
        make: no_arguments::<Reverse>,
    });
    registry.register(Entry {
        name: "censor",
        usage: "censor WORD",
        help: "hide WORD wherever it appears",
        make: make_censor,
    });
    registry
}

fn main() {
    let registry = make_registry();
    let mut pipeline: Vec<Box<dyn Plugin>> = Vec::new();
    for line in io::stdin().lock().lines() {
        let line = line.expect("stdin should be readable");
        let words: Vec<&str> = line.split_whitespace().collect();
        match words.as_slice() {
            ["list"] => {
                for entry in &registry.entries {
                    println!("{}: {}", entry.usage, entry.help);
                }
            }
            ["add", name, args @ ..] => match registry.create(name, args) {
                Ok(plugin) => {
                    println!("added {}", plugin.describe());
                    pipeline.push(plugin);
                }
                Err(error) => println!("error: {error}"),
            },
            ["pipeline"] => {
                let steps: Vec<String> = pipeline.iter().map(|plugin| plugin.describe()).collect();
                if steps.is_empty() {
                    println!("pipeline: (empty)");
                } else {
                    println!("pipeline: {}", steps.join(" | "));
                }
            }
            ["run", rest @ ..] => {
                let mut words: Vec<String> = rest.iter().map(|word| word.to_string()).collect();
                for plugin in &pipeline {
                    words = plugin.apply(words);
                }
                println!("{}", words.join(" "));
            }
            _ => {}
        }
    }
}
