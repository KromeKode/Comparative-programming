use std::io::{self, BufRead};

// An edit that knows how to do itself and how to undo itself.
trait Command {
    fn apply(&mut self, lines: &mut Vec<String>) -> Result<(), String>;
    fn undo(&mut self, lines: &mut Vec<String>);
    fn describe(&self) -> String;
}

fn check_line(lines: &[String], number: usize) -> Result<(), String> {
    if number == 0 || number > lines.len() {
        return Err(format!("no line {number}"));
    }
    Ok(())
}

struct Add {
    text: String,
}

impl Command for Add {
    fn apply(&mut self, lines: &mut Vec<String>) -> Result<(), String> {
        lines.push(self.text.clone());
        Ok(())
    }

    fn undo(&mut self, lines: &mut Vec<String>) {
        lines.pop();
    }

    fn describe(&self) -> String {
        format!("add \"{}\"", self.text)
    }
}

struct Delete {
    number: usize,
    removed: String,
}

impl Command for Delete {
    fn apply(&mut self, lines: &mut Vec<String>) -> Result<(), String> {
        check_line(lines, self.number)?;
        self.removed = lines.remove(self.number - 1);
        Ok(())
    }

    fn undo(&mut self, lines: &mut Vec<String>) {
        lines.insert(self.number - 1, std::mem::take(&mut self.removed));
    }

    fn describe(&self) -> String {
        format!("delete line {}", self.number)
    }
}

struct Replace {
    number: usize,
    text: String,
    old: String,
}

impl Command for Replace {
    fn apply(&mut self, lines: &mut Vec<String>) -> Result<(), String> {
        check_line(lines, self.number)?;
        self.old = std::mem::replace(&mut lines[self.number - 1], self.text.clone());
        Ok(())
    }

    fn undo(&mut self, lines: &mut Vec<String>) {
        lines[self.number - 1] = std::mem::take(&mut self.old);
    }

    fn describe(&self) -> String {
        format!("replace line {} with \"{}\"", self.number, self.text)
    }
}

#[derive(Default)]
struct Editor {
    lines: Vec<String>,
    done: Vec<Box<dyn Command>>,
    undone: Vec<Box<dyn Command>>,
}

impl Editor {
    fn run(&mut self, mut command: Box<dyn Command>) -> Result<(), String> {
        command.apply(&mut self.lines)?; // on an error, nothing is recorded
        self.done.push(command);
        self.undone.clear();
        Ok(())
    }

    fn undo(&mut self) -> Result<(), String> {
        let mut command = self.done.pop().ok_or("nothing to undo")?;
        command.undo(&mut self.lines);
        println!("undo: {}", command.describe());
        self.undone.push(command);
        Ok(())
    }

    fn redo(&mut self) -> Result<(), String> {
        let mut command = self.undone.pop().ok_or("nothing to redo")?;
        command.apply(&mut self.lines)?;
        println!("redo: {}", command.describe());
        self.done.push(command);
        Ok(())
    }

    fn show(&self) {
        if self.lines.is_empty() {
            println!("(empty)");
        }
        for (number, line) in self.lines.iter().enumerate() {
            println!("{}: {line}", number + 1);
        }
    }
}

fn parse_number(text: &str) -> Result<usize, String> {
    text.parse()
        .map_err(|_| format!("not a line number: {text}"))
}

fn run(editor: &mut Editor, words: &[&str]) -> Result<(), String> {
    match words {
        ["add", text @ ..] => editor.run(Box::new(Add {
            text: text.join(" "),
        })),
        ["delete", number] => editor.run(Box::new(Delete {
            number: parse_number(number)?,
            removed: String::new(),
        })),
        ["replace", number, text @ ..] => editor.run(Box::new(Replace {
            number: parse_number(number)?,
            text: text.join(" "),
            old: String::new(),
        })),
        ["undo"] => editor.undo(),
        ["redo"] => editor.redo(),
        ["show"] => {
            editor.show();
            Ok(())
        }
        _ => Ok(()),
    }
}

fn main() {
    let mut editor = Editor::default();
    for line in io::stdin().lock().lines() {
        let line = line.expect("stdin should be readable");
        let words: Vec<&str> = line.split_whitespace().collect();
        if let Err(error) = run(&mut editor, &words) {
            println!("error: {error}");
        }
    }
}
