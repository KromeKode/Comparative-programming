// Prints each line of stdin that has a match for the pattern, as grep does, with a
// matcher of its own: the pattern becomes a tree, the tree becomes a program for a
// Thompson NFA, and the NFA reads each line once, keeping the set of states it could be
// in. So a line takes time in proportion to its length times the pattern's, whatever
// the pattern is.
use std::io::{self, Read};
use std::process::ExitCode;

/// What a "\" can make a literal: the characters that mean something, inside brackets
/// or out.
const SPECIAL: &str = "\\.[]()*+?|^$-";

struct PatternError {
    at: usize, // an index into the pattern's characters
    message: &'static str,
}

/// The tree. A character, a ".", and brackets are each a set of characters: "a" is the
/// range a to a, and "." is the empty set, negated.
enum Node {
    Chars {
        ranges: Vec<(char, char)>,
        negated: bool,
    },
    Anchor(char), // '^' or '$'
    Concat(Vec<Node>),
    Alternation(Vec<Node>),
    Repeat(Box<Node>, char), // '*', '+', or '?'
}

/// Reads alternation = concat ("|" concat)*, concat = repeat*, and repeat = atom [*+?].
struct Parser {
    pattern: Vec<char>, // chars, so that an index counts characters, as columns do
    i: usize,
}

fn error<T>(at: usize, message: &'static str) -> Result<T, PatternError> {
    Err(PatternError { at, message })
}

impl Parser {
    fn peek(&self) -> Option<char> {
        self.pattern.get(self.i).copied()
    }

    fn parse(&mut self) -> Result<Node, PatternError> {
        let node = self.alternation()?;
        if self.i < self.pattern.len() {
            // alternation stops early only at a ")".
            return error(self.i, "unmatched \")\"");
        }
        Ok(node)
    }

    fn alternation(&mut self) -> Result<Node, PatternError> {
        let mut options = vec![self.concat()?];
        while self.peek() == Some('|') {
            self.i += 1;
            options.push(self.concat()?);
        }
        Ok(if options.len() == 1 {
            options.remove(0)
        } else {
            Node::Alternation(options)
        })
    }

    fn concat(&mut self) -> Result<Node, PatternError> {
        let mut items = Vec::new();
        while let Some(c) = self.peek()
            && c != '|'
            && c != ')'
        {
            items.push(self.repeat()?);
        }
        Ok(Node::Concat(items))
    }

    fn repeat(&mut self) -> Result<Node, PatternError> {
        match self.peek() {
            // At the start, or after "(", "|", "^", "$", or another "*", "+", or "?".
            Some('*' | '+' | '?') => return error(self.i, "nothing to repeat"),
            Some(c @ ('^' | '$')) => {
                self.i += 1;
                return Ok(Node::Anchor(c));
            }
            _ => {}
        }
        let item = self.atom()?;
        if let Some(c @ ('*' | '+' | '?')) = self.peek() {
            self.i += 1;
            return Ok(Node::Repeat(Box::new(item), c));
        }
        Ok(item)
    }

    fn atom(&mut self) -> Result<Node, PatternError> {
        match self.peek() {
            Some('(') => {
                let start = self.i;
                self.i += 1;
                let node = self.alternation()?;
                if self.peek() != Some(')') {
                    return error(start, "unclosed \"(\"");
                }
                self.i += 1;
                Ok(node)
            }
            Some('[') => self.brackets(),
            Some('.') => {
                self.i += 1;
                Ok(Node::Chars {
                    ranges: vec![],
                    negated: true,
                })
            }
            _ => {
                let c = self.char()?;
                Ok(Node::Chars {
                    ranges: vec![(c, c)],
                    negated: false,
                })
            }
        }
    }

    /// A character, or a "\" and the character it makes a literal.
    fn char(&mut self) -> Result<char, PatternError> {
        let c = self.pattern[self.i];
        if c != '\\' {
            self.i += 1;
            return Ok(c);
        }
        let Some(&c) = self.pattern.get(self.i + 1) else {
            return error(self.i, "trailing backslash");
        };
        if !SPECIAL.contains(c) {
            return error(self.i, "invalid escape");
        }
        self.i += 2;
        Ok(c)
    }

    fn brackets(&mut self) -> Result<Node, PatternError> {
        let start = self.i;
        self.i += 1; // the [
        let negated = self.peek() == Some('^');
        if negated {
            self.i += 1;
        }
        if self.peek() == Some(']') {
            return error(start, "empty brackets");
        }
        let mut ranges = Vec::new();
        loop {
            match self.peek() {
                None => return error(start, "unclosed \"[\""),
                Some(']') => break,
                Some(_) => {}
            }
            let at = self.i;
            let low = self.char()?;
            let mut high = low;
            // A "-" makes a range, unless it's last: "[a-]" is "a" and "-".
            if self.peek() == Some('-') && self.pattern.get(self.i + 1).is_some_and(|&c| c != ']') {
                self.i += 1;
                high = self.char()?;
                if high < low {
                    return error(at, "range out of order");
                }
            }
            ranges.push((low, high));
        }
        self.i += 1; // the ]
        Ok(Node::Chars { ranges, negated })
    }
}

/// The program. Chars reads a character, if it's in the set, and goes on to the next
/// instruction; Split goes on to both of its two; Jump goes on to its one; Start and End
/// go on if the position is the start or the end of the line; and Match is a match.
enum Instruction {
    Chars {
        ranges: Vec<(char, char)>,
        negated: bool,
    },
    Split(usize, usize),
    Jump(usize),
    Start,
    End,
    Match,
}

impl Instruction {
    fn accepts(&self, c: char) -> bool {
        match self {
            Instruction::Chars { ranges, negated } => {
                ranges.iter().any(|&(low, high)| (low..=high).contains(&c)) != *negated
            }
            _ => false,
        }
    }
}

/// Appends node's instructions to the program. They refer to each other by index.
fn compile(node: &Node, program: &mut Vec<Instruction>) {
    let start = program.len();
    match node {
        Node::Chars { ranges, negated } => program.push(Instruction::Chars {
            ranges: ranges.clone(),
            negated: *negated,
        }),
        Node::Anchor('^') => program.push(Instruction::Start),
        Node::Anchor(_) => program.push(Instruction::End),
        Node::Concat(items) => {
            for item in items {
                compile(item, program);
            }
        }
        Node::Alternation(options) => {
            // split L1, L2; L1: first; jump END; L2: split L3, L4; L3: second; ...
            let (last, rest) = options.split_last().expect("two options or more");
            let mut jumps = Vec::new();
            for option in rest {
                let split = program.len();
                program.push(Instruction::Split(split + 1, 0));
                compile(option, program);
                jumps.push(program.len());
                program.push(Instruction::Jump(0));
                program[split] = Instruction::Split(split + 1, program.len());
            }
            compile(last, program);
            let end = program.len();
            for jump in jumps {
                program[jump] = Instruction::Jump(end);
            }
        }
        Node::Repeat(item, '*') => {
            // L1: split L2, L3; L2: item; jump L1; L3:
            program.push(Instruction::Split(start + 1, 0));
            compile(item, program);
            program.push(Instruction::Jump(start));
            program[start] = Instruction::Split(start + 1, program.len());
        }
        Node::Repeat(item, '+') => {
            // L1: item; split L1, L2; L2:
            compile(item, program);
            program.push(Instruction::Split(start, program.len() + 1));
        }
        Node::Repeat(item, _) => {
            // split L1, L2; L1: item; L2:
            program.push(Instruction::Split(start + 1, 0));
            compile(item, program);
            program[start] = Instruction::Split(start + 1, program.len());
        }
    }
}

/// The Chars instructions that pcs lead to without reading a character, at the
/// position at, and whether one of them leads to Match.
fn follow(program: &[Instruction], pcs: &[usize], at: usize, len: usize) -> (Vec<usize>, bool) {
    let mut waiting = Vec::new();
    let mut seen = vec![false; program.len()];
    let mut stack = pcs.to_vec();
    while let Some(pc) = stack.pop() {
        if seen[pc] {
            continue; // so that a loop such as (a*)* doesn't go around forever
        }
        seen[pc] = true;
        match program[pc] {
            Instruction::Chars { .. } => waiting.push(pc),
            Instruction::Split(x, y) => stack.extend([y, x]),
            Instruction::Jump(x) => stack.push(x),
            Instruction::Start if at == 0 => stack.push(pc + 1),
            Instruction::End if at == len => stack.push(pc + 1),
            Instruction::Start | Instruction::End => {}
            Instruction::Match => return (vec![], true),
        }
    }
    (waiting, false)
}

/// Whether a match starts anywhere in the line: instruction 0 starts at each position.
fn search(program: &[Instruction], line: &[char]) -> bool {
    let mut waiting = Vec::new();
    for at in 0..=line.len() {
        waiting.push(0);
        let (reached, matched) = follow(program, &waiting, at, line.len());
        if matched {
            return true;
        }
        waiting = match line.get(at) {
            Some(&c) => reached
                .into_iter()
                .filter(|&pc| program[pc].accepts(c))
                .map(|pc| pc + 1)
                .collect(),
            None => vec![],
        };
    }
    false
}

fn main() -> ExitCode {
    let args: Vec<String> = std::env::args().skip(1).collect();
    let [pattern] = args.as_slice() else {
        eprintln!("usage: rematch PATTERN");
        return ExitCode::from(2);
    };
    let mut parser = Parser {
        pattern: pattern.chars().collect(),
        i: 0,
    };
    let tree = match parser.parse() {
        Ok(tree) => tree,
        Err(err) => {
            eprintln!("rematch: column {}: {}", err.at + 1, err.message);
            return ExitCode::from(2);
        }
    };
    let mut program = Vec::new();
    compile(&tree, &mut program);
    program.push(Instruction::Match);
    let mut text = String::new();
    if let Err(err) = io::stdin().read_to_string(&mut text) {
        eprintln!("rematch: {err}");
        return ExitCode::from(2);
    }
    let mut lines: Vec<&str> = text.split('\n').collect();
    if lines.last() == Some(&"") {
        lines.pop(); // what comes after the last "\n", or the empty input
    }
    let mut out = String::new();
    for line in lines {
        if search(&program, &line.chars().collect::<Vec<_>>()) {
            out += line;
            out.push('\n');
        }
    }
    print!("{out}");
    if out.is_empty() {
        ExitCode::from(1)
    } else {
        ExitCode::SUCCESS
    }
}
