use std::cell::Cell;
use std::io::{self, BufRead};

thread_local! {
    static ON: Cell<bool> = const { Cell::new(false) };
    static DEPTH: Cell<usize> = const { Cell::new(0) };
    static CALLS: Cell<u64> = const { Cell::new(0) };
}

fn say(text: &str) {
    if ON.get() {
        println!("{}{text}", "  ".repeat(DEPTH.get()));
    }
}

fn trace(call: String, body: impl FnOnce() -> i64) -> i64 {
    CALLS.set(CALLS.get() + 1);
    say(&call);
    DEPTH.set(DEPTH.get() + 1);
    let result = body();
    DEPTH.set(DEPTH.get() - 1);
    say(&format!("-> {result}"));
    result
}

type Call = fn(&[i64]) -> i64;

// traced! takes ordinary function definitions. Each one becomes a function of
// the same name that traces the call and runs the original body, so the
// recursive calls in the bodies are traced too. The macro also builds
// FUNCTIONS: each name, its number of parameters, and a function that calls
// it with a slice of arguments.
macro_rules! traced {
    ($(fn $name:ident($($arg:ident: i64),*) -> i64 $body:block)*) => {
        $(
            fn $name($($arg: i64),*) -> i64 {
                let args: &[String] = &[$($arg.to_string()),*];
                trace(format!("{}({})", stringify!($name), args.join(", ")), || $body)
            }
        )*

        const FUNCTIONS: &[(&str, usize, Call)] = &[$((
            stringify!($name),
            [$(stringify!($arg)),*].len(),
            |args| {
                // The parameter names work as a pattern too.
                let &[$($arg),*] = args else { unreachable!() };
                $name($($arg),*)
            },
        )),*];
    };
}

traced! {
    fn fib(n: i64) -> i64 {
        if n < 2 { n } else { fib(n - 1) + fib(n - 2) }
    }

    fn gcd(a: i64, b: i64) -> i64 {
        if b == 0 { a } else { gcd(b, a % b) }
    }

    fn power(base: i64, exponent: i64) -> i64 {
        if exponent == 0 {
            return 1;
        }
        if exponent % 2 == 0 {
            let half = power(base, exponent / 2);
            return half * half;
        }
        base * power(base, exponent - 1)
    }
}

fn plural(count: u64, word: &str) -> String {
    format!("{count} {word}{}", if count == 1 { "" } else { "s" })
}

fn run(line: &str) {
    let mut words = line.split_whitespace();
    let (command, name) = (words.next().unwrap_or(""), words.next().unwrap_or(""));
    let args: Vec<i64> = words.map(|word| word.parse().unwrap_or(0)).collect();
    if command != "trace" && command != "count" {
        println!("unknown command: {command}");
        return;
    }
    let Some(&(_, arity, call)) = FUNCTIONS.iter().find(|entry| entry.0 == name) else {
        println!("unknown function: {name}");
        return;
    };
    if args.len() != arity {
        let takes = plural(arity as u64, "argument");
        println!("{name} takes {takes}, got {}", args.len());
        return;
    }
    ON.set(command == "trace");
    DEPTH.set(0);
    CALLS.set(0);
    let result = call(&args);
    let shown: Vec<String> = args.iter().map(|arg| arg.to_string()).collect();
    let calls = plural(CALLS.get(), "call");
    println!("{name}({}) = {result} in {calls}", shown.join(", "));
}

fn main() {
    for line in io::stdin().lock().lines() {
        let line = line.expect("stdin should be readable");
        if !line.trim().is_empty() {
            run(&line);
        }
    }
}
