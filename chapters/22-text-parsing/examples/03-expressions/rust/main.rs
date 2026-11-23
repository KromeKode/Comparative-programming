//! Reads arithmetic, one expression on each line, such as 1 + 2 * 3, and prints each with
//! its parentheses written out, and its value: (1 + (2 * 3)) = 7. The numbers are whole
//! and fit in 64 bits; the operators are + - * / % ^ and unary -, with ( and ). / and %
//! round toward zero, as in C. An error is printed with its line and a caret under its
//! column, the next line is read, and at the end, the exit status is 1.
use std::io::{self, Read};
use std::process::ExitCode;

struct CalcError {
    column: usize, // counted in characters, from 1
    message: String,
}

fn error<T>(column: usize, message: impl Into<String>) -> Result<T, CalcError> {
    Err(CalcError {
        column,
        message: message.into(),
    })
}

// The lexer turns the text into tokens: numbers, and operators and parentheses, each with
// its column.
#[derive(Clone, Copy, PartialEq)]
enum Kind {
    Number(i64),
    Op(char), // an operator or a parenthesis
    End,      // after the last token
}

#[derive(Clone)]
struct Token {
    kind: Kind,
    text: String,
    column: usize,
}

fn describe(token: &Token) -> String {
    match token.kind {
        Kind::End => "the end of the line".to_string(),
        _ => format!("\"{}\"", token.text),
    }
}

fn tokens(line: &str) -> Result<Vec<Token>, CalcError> {
    // A Vec of chars, so that an index counts code points, and i + 1 is the column.
    let chars: Vec<char> = line.chars().collect();
    let mut found = Vec::new();
    let mut i = 0;
    while i < chars.len() {
        let c = chars[i];
        // is_ascii_digit, not is_numeric, which is true for "٣" and "²".
        if c == ' ' || c == '\t' {
            i += 1;
        } else if c.is_ascii_digit() {
            let start = i;
            while i < chars.len() && chars[i].is_ascii_digit() {
                i += 1;
            }
            let text: String = chars[start..i].iter().collect();
            // Only digits, so parse fails only if the number doesn't fit.
            let Ok(value) = text.parse() else {
                return error(start + 1, "number too large for 64 bits");
            };
            found.push(Token {
                kind: Kind::Number(value),
                text,
                column: start + 1,
            });
        } else if "+-*/%^()".contains(c) {
            found.push(Token {
                kind: Kind::Op(c),
                text: c.to_string(),
                column: i + 1,
            });
            i += 1;
        } else {
            return error(i + 1, format!("unexpected \"{c}\""));
        }
    }
    found.push(Token {
        kind: Kind::End,
        text: String::new(),
        column: chars.len() + 1,
    });
    Ok(found)
}

// The parser turns the tokens into a tree, whose nodes are of three kinds. A Box holds a
// node on the heap, since an Expr can't hold another Expr inside itself.
enum Expr {
    Number(i64),
    // column is where the operator is, for an error such as overflow.
    Negate {
        operand: Box<Expr>,
        column: usize,
    },
    Binary {
        op: char,
        left: Box<Expr>,
        right: Box<Expr>,
        column: usize,
    },
}

/// How tightly each operator holds the expressions on its left and right: the higher,
/// the tighter. Left is less than right for left-associative operators, so 1 - 2 - 3 is
/// (1 - 2) - 3, and more for ^, so 2 ^ 3 ^ 2 is 2 ^ (3 ^ 2).
fn infix(op: char) -> Option<(u8, u8)> {
    match op {
        '+' | '-' => Some((1, 2)),
        '*' | '/' | '%' => Some((3, 4)),
        '^' => Some((8, 7)),
        _ => None,
    }
}

/// Unary minus holds less tightly than ^, so -2 ^ 2 is -(2 ^ 2), and more than *.
const PREFIX: u8 = 5;

struct Parser {
    tokens: Vec<Token>,
    at: usize,
}

impl Parser {
    fn next(&mut self) -> Token {
        let token = self.tokens[self.at].clone();
        if token.kind != Kind::End {
            self.at += 1;
        }
        token
    }

    /// Parses the longest expression whose operators hold at least min_power.
    fn expression(&mut self, min_power: u8) -> Result<Expr, CalcError> {
        let token = self.next();
        let mut left = match token.kind {
            Kind::Number(value) => Expr::Number(value),
            Kind::Op('-') => {
                let operand = Box::new(self.expression(PREFIX)?);
                Expr::Negate {
                    operand,
                    column: token.column,
                }
            }
            Kind::Op('(') => {
                let inside = self.expression(0)?;
                let close = self.next();
                if close.kind != Kind::Op(')') {
                    return error(
                        close.column,
                        format!("expected \")\", found {}", describe(&close)),
                    );
                }
                inside
            }
            _ => {
                let found = describe(&token);
                return error(
                    token.column,
                    format!("expected a number, \"(\", or \"-\", found {found}"),
                );
            }
        };
        loop {
            let token = &self.tokens[self.at];
            let Kind::Op(op) = token.kind else {
                return Ok(left);
            };
            let Some((left_power, right_power)) = infix(op) else {
                return Ok(left);
            };
            if left_power < min_power {
                return Ok(left);
            }
            let column = self.next().column;
            let right = Box::new(self.expression(right_power)?);
            left = Expr::Binary {
                op,
                left: Box::new(left),
                right,
                column,
            };
        }
    }

    fn parse(line: &str) -> Result<Expr, CalcError> {
        let mut parser = Parser {
            tokens: tokens(line)?,
            at: 0,
        };
        let expr = parser.expression(0)?;
        let token = parser.next();
        if token.kind != Kind::End {
            return error(
                token.column,
                format!("expected an operator, found {}", describe(&token)),
            );
        }
        Ok(expr)
    }
}

fn show(expr: &Expr) -> String {
    match expr {
        Expr::Number(value) => value.to_string(),
        Expr::Negate { operand, .. } => format!("(-{})", show(operand)),
        Expr::Binary {
            op, left, right, ..
        } => format!("({} {op} {})", show(left), show(right)),
    }
}

/// The value, or an overflow error at column if checked arithmetic found none.
fn fit(value: Option<i64>, column: usize) -> Result<i64, CalcError> {
    value.map_or_else(|| error(column, "overflow"), Ok)
}

/// Squares the base for each bit of the exponent, and stops at the first overflow.
/// i64's checked_pow takes the exponent as a u32.
fn power(mut base: i64, mut exponent: i64, column: usize) -> Result<i64, CalcError> {
    let mut result: i64 = 1;
    while exponent > 0 {
        if exponent % 2 == 1 {
            result = fit(result.checked_mul(base), column)?;
        }
        exponent /= 2;
        if exponent > 0 {
            base = fit(base.checked_mul(base), column)?;
        }
    }
    Ok(result)
}

fn evaluate(expr: &Expr) -> Result<i64, CalcError> {
    match expr {
        Expr::Number(value) => Ok(*value),
        Expr::Negate { operand, column } => fit(evaluate(operand)?.checked_neg(), *column),
        Expr::Binary {
            op,
            left,
            right,
            column,
        } => {
            let (a, b) = (evaluate(left)?, evaluate(right)?);
            let column = *column;
            if (*op == '/' || *op == '%') && b == 0 {
                return error(column, "division by zero");
            }
            // checked_ gives None where the answer doesn't fit in an i64; / and % round
            // toward zero. i64::MIN % -1 overflows, but its remainder is 0, as
            // wrapping_rem says.
            match op {
                '+' => fit(a.checked_add(b), column),
                '-' => fit(a.checked_sub(b), column),
                '*' => fit(a.checked_mul(b), column),
                '/' => fit(a.checked_div(b), column),
                '%' => Ok(a.wrapping_rem(b)),
                _ if b < 0 => error(column, "negative exponent"),
                _ => power(a, b, column),
            }
        }
    }
}

fn main() -> ExitCode {
    let mut text = String::new();
    if let Err(err) = io::stdin().read_to_string(&mut text) {
        eprintln!("calc: {err}");
        return ExitCode::FAILURE;
    }
    let mut status = ExitCode::SUCCESS;
    // lines() splits at "\n" and removes a "\r" before it.
    for (index, line) in text.lines().enumerate() {
        if line.trim_matches([' ', '\t']).is_empty() {
            continue;
        }
        match Parser::parse(line).and_then(|expr| Ok((show(&expr), evaluate(&expr)?))) {
            Ok((shown, value)) => println!("{shown} = {value}"),
            Err(err) => {
                // The caret goes under the column. A tab before it stays a tab, so that
                // the caret lines up with the text whatever a tab's width is.
                let pad: String = line
                    .chars()
                    .take(err.column - 1)
                    .map(|c| if c == '\t' { '\t' } else { ' ' })
                    .collect();
                eprintln!(
                    "calc: line {}, column {}: {}",
                    index + 1,
                    err.column,
                    err.message
                );
                eprintln!("  {line}\n  {pad}^");
                status = ExitCode::FAILURE;
            }
        }
    }
    status
}
