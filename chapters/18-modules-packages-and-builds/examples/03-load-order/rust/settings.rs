use std::sync::LazyLock;

// Rust runs no code before main. A LazyLock runs its closure the first time it is
// used, once, even if several threads use it at the same moment.
pub static GREETING: LazyLock<String> = LazyLock::new(|| {
    println!("settings: loading");
    String::from("Hello")
});
