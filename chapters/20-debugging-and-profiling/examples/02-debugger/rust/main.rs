// Evaluates arithmetic with +, -, *, and parentheses: each argument, or each line of
// input if there are none. Section 20.5 runs it in a debugger, to watch the parser
// call itself. It expects well-formed expressions, and doesn't check.
use std::io::BufRead;

/// A recursive descent parser, with one method for each rule of the grammar.
struct Parser<'a> {
    text: &'a [u8],
    pos: usize,
}

impl Parser<'_> {
    /// Skips spaces, and gives the next byte, or `None` at the end.
    fn peek(&mut self) -> Option<u8> {
        while self.text.get(self.pos) == Some(&b' ') {
            self.pos += 1;
        }
        self.text.get(self.pos).copied()
    }

    // expr = term { ("+" | "-") term }
    fn expr(&mut self) -> i64 {
        let mut value = self.term();
        while let Some(op @ (b'+' | b'-')) = self.peek() {
            self.pos += 1;
            let right = self.term();
            value = if op == b'+' {
                value + right
            } else {
                value - right
            };
        }
        value
    }

    // term = factor { "*" factor }
    fn term(&mut self) -> i64 {
        let mut value = self.factor();
        while self.peek() == Some(b'*') {
            self.pos += 1;
            value *= self.factor();
        }
        value
    }

    // factor = number | "(" expr ")"
    fn factor(&mut self) -> i64 {
        if self.peek() == Some(b'(') {
            self.pos += 1;
            let value = self.expr();
            self.peek();
            self.pos += 1; // the ")"
            return value;
        }
        let mut value = 0;
        while let Some(&digit @ b'0'..=b'9') = self.text.get(self.pos) {
            value = value * 10 + i64::from(digit - b'0');
            self.pos += 1;
        }
        value
    }
}

fn evaluate(text: &str) -> i64 {
    Parser {
        text: text.as_bytes(),
        pos: 0,
    }
    .expr()
}

fn main() {
    let args: Vec<String> = std::env::args().skip(1).collect();
    for text in &args {
        println!("{}", evaluate(text));
    }
    if args.is_empty() {
        for line in std::io::stdin().lock().lines() {
            println!("{}", evaluate(&line.unwrap()));
        }
    }
}
