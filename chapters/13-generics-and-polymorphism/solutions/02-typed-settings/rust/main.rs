use std::io::{self, BufRead};

/// One setting whose value is a T. It knows how to read a T from text and how
/// to show one, so T needs no traits of its own for that.
struct Setting<T> {
    name: &'static str,
    default: T,
    value: T,
    parse: fn(&str) -> Option<T>,
    show: fn(&T) -> String,
    needs: &'static str,
}

impl<T: Clone> Setting<T> {
    fn new(
        name: &'static str,
        default: T,
        parse: fn(&str) -> Option<T>,
        show: fn(&T) -> String,
        needs: &'static str,
    ) -> Self {
        let value = default.clone();
        Setting {
            name,
            default,
            value,
            parse,
            show,
            needs,
        }
    }
}

/// What the command loop needs from a setting, without knowing its T.
trait AnySetting {
    fn name(&self) -> &str;
    fn set_text(&mut self, text: &str) -> Result<(), String>;
    fn reset(&mut self);
    fn describe(&self) -> String;
    fn is_default(&self) -> bool;
}

// One impl for every Setting<T> whose T can be cloned and compared.
impl<T: Clone + PartialEq> AnySetting for Setting<T> {
    fn name(&self) -> &str {
        self.name
    }

    fn set_text(&mut self, text: &str) -> Result<(), String> {
        let value = (self.parse)(text)
            .ok_or_else(|| format!("{} needs {}, not \"{text}\"", self.name, self.needs))?;
        self.value = value;
        Ok(())
    }

    fn reset(&mut self) {
        self.value = self.default.clone();
    }

    fn describe(&self) -> String {
        format!("{} = {}", self.name, (self.show)(&self.value))
    }

    fn is_default(&self) -> bool {
        self.value == self.default
    }
}

fn parse_int(text: &str) -> Option<i32> {
    // str::parse would also take "+5", so check the form first.
    let digits = text.strip_prefix('-').unwrap_or(text);
    if digits.is_empty() || !digits.bytes().all(|b| b.is_ascii_digit()) {
        return None;
    }
    text.parse().ok()
}

fn parse_bool(text: &str) -> Option<bool> {
    match text {
        "true" => Some(true),
        "false" => Some(false),
        _ => None,
    }
}

// Typed fields: volume.value is an i32 and muted.value a bool, with no casts.
struct Settings {
    volume: Setting<i32>,
    muted: Setting<bool>,
    name: Setting<String>,
    tags: Setting<Vec<String>>,
}

impl Settings {
    fn new() -> Self {
        Settings {
            volume: Setting::new("volume", 5, parse_int, |n| n.to_string(), "a whole number"),
            muted: Setting::new(
                "muted",
                false,
                parse_bool,
                |b| b.to_string(),
                "true or false",
            ),
            name: Setting::new(
                "name",
                "guest".into(),
                |t| Some(t.into()),
                |t| t.clone(),
                "",
            ),
            tags: Setting::new(
                "tags",
                Vec::new(),
                |t| Some(t.split_whitespace().map(String::from).collect()),
                |tags| format!("[{}]", tags.join(", ")),
                "",
            ),
        }
    }

    // The same settings as trait objects, so a loop can treat them alike.
    fn all(&self) -> [&dyn AnySetting; 4] {
        [&self.volume, &self.muted, &self.name, &self.tags]
    }

    fn find(&mut self, name: &str) -> Result<&mut dyn AnySetting, String> {
        let all: [&mut dyn AnySetting; 4] = [
            &mut self.volume,
            &mut self.muted,
            &mut self.name,
            &mut self.tags,
        ];
        all.into_iter()
            .find(|setting| setting.name() == name)
            .ok_or_else(|| format!("no setting named \"{name}\""))
    }

    fn summary(&self) -> String {
        let name = &self.name.value;
        if self.muted.value {
            format!("{name} has muted the sound")
        } else {
            format!(
                "{name} hears the sound at {}%",
                i64::from(self.volume.value) * 10
            )
        }
    }
}

fn run(settings: &mut Settings, words: &[&str]) -> Result<(), String> {
    match words {
        ["set", name, value @ ..] => {
            let setting = settings.find(name)?;
            setting.set_text(&value.join(" "))?;
            println!("{}", setting.describe());
        }
        ["get", name] => println!("{}", settings.find(name)?.describe()),
        ["reset", name] => {
            let setting = settings.find(name)?;
            setting.reset();
            println!("{}", setting.describe());
        }
        ["list"] => {
            for setting in settings.all() {
                let mark = if setting.is_default() {
                    " (default)"
                } else {
                    ""
                };
                println!("{}{mark}", setting.describe());
            }
        }
        ["summary"] => println!("{}", settings.summary()),
        _ => {}
    }
    Ok(())
}

fn main() {
    let mut settings = Settings::new();
    for line in io::stdin().lock().lines() {
        let line = line.expect("read stdin");
        let words: Vec<&str> = line.split_whitespace().collect();
        if let Err(message) = run(&mut settings, &words) {
            println!("error: {message}");
        }
    }
}
