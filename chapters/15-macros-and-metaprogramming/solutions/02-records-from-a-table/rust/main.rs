use std::io::{self, Read};

// One parser and one printer per type a field can have. The macro below
// calls them through the field's type, so the compiler picks the right one.
trait Field: Sized {
    fn parse(text: &str) -> Result<Self, String>;
    fn show(&self) -> String;
}

impl Field for String {
    fn parse(text: &str) -> Result<Self, String> {
        Ok(text.to_string())
    }

    fn show(&self) -> String {
        format!("\"{self}\"")
    }
}

impl Field for i64 {
    fn parse(text: &str) -> Result<Self, String> {
        let digits = text.strip_prefix('-').unwrap_or(text);
        if digits.is_empty() || !digits.bytes().all(|b| b.is_ascii_digit()) {
            return Err(format!("\"{text}\" is not a whole number"));
        }
        text.parse()
            .map_err(|_| format!("\"{text}\" is not a whole number"))
    }

    fn show(&self) -> String {
        self.to_string()
    }
}

impl Field for bool {
    fn parse(text: &str) -> Result<Self, String> {
        match text {
            "true" => Ok(true),
            "false" => Ok(false),
            _ => Err(format!("\"{text}\" is not true or false")),
        }
    }

    fn show(&self) -> String {
        self.to_string()
    }
}

// record! takes a struct definition and writes it back out, with Default
// derived, and adds the field names, a set method that picks a field by
// name, and a describe method that shows every field in order.
macro_rules! record {
    (struct $name:ident { $($field:ident: $type:ty),* $(,)? }) => {
        #[derive(Default)]
        struct $name {
            $($field: $type),*
        }

        impl $name {
            const FIELDS: &[&str] = &[$(stringify!($field)),*];

            fn set(&mut self, name: &str, text: &str) -> Result<(), String> {
                match name {
                    $(stringify!($field) => {
                        self.$field = <$type as Field>::parse(text)
                            .map_err(|problem| format!("{name}: {problem}"))?;
                    })*
                    _ => return Err(format!("unknown column: {name}")),
                }
                Ok(())
            }

            fn describe(&self) -> String {
                let parts = [$(format!("{}={}", stringify!($field), self.$field.show())),*];
                format!("{}({})", stringify!($name), parts.join(", "))
            }
        }
    };
}

// The fields are written once, here.
record! {
    struct Book {
        title: String,
        author: String,
        year: i64,
        pages: i64,
        in_print: bool,
    }
}

fn split(line: &str) -> Vec<&str> {
    line.split(',').map(str::trim).collect()
}

fn plural(count: usize, word: &str) -> String {
    format!("{count} {word}{}", if count == 1 { "" } else { "s" })
}

fn main() {
    let mut input = String::new();
    io::stdin()
        .read_to_string(&mut input)
        .expect("stdin should be readable");
    let mut lines = input.lines().filter(|line| !line.trim().is_empty());
    let Some(header) = lines.next() else { return };
    let columns = split(header);
    for (i, name) in columns.iter().enumerate() {
        if !Book::FIELDS.contains(name) {
            println!("unknown column: {name}");
            return;
        }
        if columns[..i].contains(name) {
            println!("repeated column: {name}");
            return;
        }
    }

    let (mut books, mut bad) = (0, 0);
    for (row, line) in lines.enumerate() {
        let values = split(line);
        let result = if values.len() != columns.len() {
            Err(format!(
                "expected {} values, got {}",
                columns.len(),
                values.len()
            ))
        } else {
            let mut book = Book::default();
            columns
                .iter()
                .zip(&values)
                .try_for_each(|(name, text)| book.set(name, text))
                .map(|()| book)
        };
        match result {
            Ok(book) => {
                println!("{}", book.describe());
                books += 1;
            }
            Err(problem) => {
                println!("row {}: {problem}", row + 1);
                bad += 1;
            }
        }
    }
    println!("{}, {}", plural(books, "book"), plural(bad, "bad row"));
}
