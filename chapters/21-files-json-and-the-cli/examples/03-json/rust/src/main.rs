//! Reads a JSON file of books and prints a report on them, as JSON: how many there are,
//! the oldest, the books with each tag, and the books with none. serde's derive writes
//! the code that turns JSON into a `Library` and a `Report` into JSON, checking every
//! field's type on the way in.
use serde::{Deserialize, Serialize};
use std::collections::BTreeMap;
use std::process::ExitCode;

#[derive(Deserialize)]
struct Library {
    books: Vec<Book>,
}

#[derive(Deserialize)]
struct Book {
    title: String,
    author: String,
    year: i64,
    #[serde(default)] // an empty Vec when tags is missing
    tags: Vec<String>,
}

// The fields are written in this order, under these names.
#[derive(Serialize)]
struct Report<'a> {
    count: usize,
    oldest: Option<String>,                // None is written as null
    tags: BTreeMap<&'a str, Vec<&'a str>>, // a BTreeMap keeps its keys in order
    untagged: Vec<&'a str>,
}

fn report(books: &[Book]) -> Report<'_> {
    let mut tags: BTreeMap<&str, Vec<&str>> = BTreeMap::new();
    for book in books {
        for tag in &book.tags {
            tags.entry(tag).or_default().push(&book.title);
        }
    }
    Report {
        count: books.len(),
        // min_by_key gives the first of the oldest, when two have the same year.
        oldest: books
            .iter()
            .min_by_key(|book| book.year)
            .map(|book| format!("{}, by {}", book.title, book.author)),
        tags,
        untagged: books
            .iter()
            .filter(|book| book.tags.is_empty())
            .map(|book| book.title.as_str())
            .collect(),
    }
}

fn main() -> ExitCode {
    let args: Vec<String> = std::env::args().collect();
    let [_, path] = args.as_slice() else {
        eprintln!("usage: books FILE");
        return ExitCode::from(2);
    };
    let text = match std::fs::read_to_string(path) {
        Ok(text) => text,
        Err(error) => {
            eprintln!("books: {path}: {error}");
            return ExitCode::FAILURE;
        }
    };
    // from_str's type, Library, comes from the variable's: serde reads the JSON into it.
    let library: Library = match serde_json::from_str(&text) {
        Ok(library) => library,
        Err(error) => {
            // Such as "missing field `year` at line 1 column 49".
            eprintln!("books: {path}: {error}");
            return ExitCode::FAILURE;
        }
    };
    // to_string_pretty can only fail for a map whose keys aren't strings.
    let json = serde_json::to_string_pretty(&report(&library.books)).expect("a report");
    println!("{json}");
    ExitCode::SUCCESS
}
