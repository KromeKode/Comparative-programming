use std::collections::HashSet;

// A macro that takes a whole struct definition, writes it back out as it
// was, and adds a list of the field names. $(#[$meta:meta])* passes the
// attributes through, so the derive below still applies.
macro_rules! record {
    ($(#[$meta:meta])* struct $name:ident { $($field:ident: $type:ty),* $(,)? }) => {
        $(#[$meta])*
        struct $name {
            $($field: $type),*
        }

        impl $name {
            const FIELDS: &[&str] = &[$(stringify!($field)),*];
        }
    };
}

record! {
    // derive asks the compiler to write each trait's methods from the fields:
    // Debug prints them, PartialEq and Eq compare every field, PartialOrd and
    // Ord compare them in order, Hash lets a HashSet use the struct, and
    // Clone copies it.
    #[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Ord, Hash)]
    struct Book {
        title: String,
        author: String,
        year: u32,
        in_print: bool,
    }
}

fn main() {
    println!("fields: {}", Book::FIELDS.join(" "));

    let dune = Book {
        title: "Dune".to_string(),
        author: "Frank Herbert".to_string(),
        year: 1965,
        in_print: true,
    };
    println!("dune: {dune:?}");

    let copy = dune.clone();
    println!("copy == dune: {}", copy == dune);

    // ..dune.clone() fills in every field that isn't named.
    let reissue = Book {
        year: 1966,
        ..dune.clone()
    };
    println!("reissue: {reissue:?}");
    println!("reissue == dune: {}", reissue == dune);
    println!("dune < reissue: {}", dune < reissue);

    let distinct: HashSet<Book> = [dune, copy, reissue].into_iter().collect();
    println!("distinct books: {}", distinct.len());
}
