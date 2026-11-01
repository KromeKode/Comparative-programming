use std::cell::{Cell, RefCell};
use std::collections::HashMap;
use std::io::{self, BufRead};

const MAX_OPEN: usize = 2;
const MAX_LINES: usize = 8;

#[derive(Default)]
struct File {
    lines: Vec<String>,
    locked: bool,
}

// Handles borrow the disk and change it through Cell and RefCell, so several
// handles can be open on one disk at once.
#[derive(Default)]
struct Disk {
    files: RefCell<HashMap<String, File>>,
    open_count: Cell<usize>,
}

impl Disk {
    fn lines_used(&self) -> usize {
        self.files.borrow().values().map(|f| f.lines.len()).sum()
    }

    // The Handle borrows the disk, so it cannot outlive it. Dropping it closes
    // the file.
    fn open(&self, name: &str, writing: bool) -> Result<Handle<'_>, String> {
        if self.open_count.get() == MAX_OPEN {
            return Err("too many open files".to_string());
        }
        let mut files = self.files.borrow_mut();
        if writing {
            let file = files.entry(name.to_string()).or_default();
            if file.locked {
                return Err(format!("\"{name}\" is read-only"));
            }
            file.lines.clear();
        } else if !files.contains_key(name) {
            return Err(format!("no file \"{name}\""));
        }
        self.open_count.set(self.open_count.get() + 1);
        println!("open {name}");
        Ok(Handle {
            disk: self,
            name: name.to_string(),
        })
    }
}

struct Handle<'a> {
    disk: &'a Disk,
    name: String,
}

impl Handle<'_> {
    fn lines(&self) -> Vec<String> {
        self.disk.files.borrow()[&self.name].lines.clone()
    }

    fn write(&self, line: &str) -> Result<(), String> {
        if self.disk.lines_used() == MAX_LINES {
            return Err("disk full".to_string());
        }
        let mut files = self.disk.files.borrow_mut();
        files
            .get_mut(&self.name)
            .unwrap()
            .lines
            .push(line.to_string());
        Ok(())
    }
}

impl Drop for Handle<'_> {
    fn drop(&mut self) {
        self.disk.open_count.set(self.disk.open_count.get() - 1);
        println!("close {}", self.name);
    }
}

fn lines(count: usize) -> String {
    if count == 1 {
        "1 line".to_string()
    } else {
        format!("{count} lines")
    }
}

// Each `?` returns early, and the handles opened so far are dropped, which
// closes them, last opened first.
fn write_file(disk: &Disk, name: &str, words: &[&str]) -> Result<(), String> {
    {
        let target = disk.open(name, true)?;
        for word in words {
            target.write(word)?;
        }
    }
    println!("wrote {}", lines(words.len()));
    Ok(())
}

fn cat_file(disk: &Disk, name: &str) -> Result<(), String> {
    let source = disk.open(name, false)?;
    for line in source.lines() {
        println!("  {line}");
    }
    Ok(())
}

fn copy_file(disk: &Disk, from: &str, to: &str) -> Result<(), String> {
    if from == to {
        return Err(format!("cannot copy \"{from}\" onto itself"));
    }
    let mut copied = 0;
    {
        let source = disk.open(from, false)?;
        let target = disk.open(to, true)?;
        for line in source.lines() {
            target.write(&line)?;
            copied += 1;
        }
    }
    println!("copied {}", lines(copied));
    Ok(())
}

fn lock_file(disk: &Disk, name: &str) -> Result<(), String> {
    match disk.files.borrow_mut().get_mut(name) {
        Some(file) => file.locked = true,
        None => return Err(format!("no file \"{name}\"")),
    }
    println!("locked {name}");
    Ok(())
}

fn takes(command: &str, want: usize, got: usize) -> Result<(), String> {
    if got == want {
        return Ok(());
    }
    let noun = if want == 1 { "name" } else { "names" };
    Err(format!("{command} takes {want} {noun}, got {got}"))
}

fn run(disk: &Disk, command: &str, args: &[&str]) -> Result<(), String> {
    match command {
        "write" => match args.split_first() {
            Some((name, words)) => write_file(disk, name, words),
            None => Err("write needs a file name".to_string()),
        },
        "cat" => {
            takes(command, 1, args.len())?;
            cat_file(disk, args[0])
        }
        "copy" => {
            takes(command, 2, args.len())?;
            copy_file(disk, args[0], args[1])
        }
        "lock" => {
            takes(command, 1, args.len())?;
            lock_file(disk, args[0])
        }
        _ => Err(format!("unknown command \"{command}\"")),
    }
}

fn main() {
    let disk = Disk::default();
    for line in io::stdin().lock().lines() {
        let line = line.expect("failed to read stdin");
        let parts: Vec<&str> = line.split_whitespace().collect();
        let Some((command, args)) = parts.split_first() else {
            continue;
        };
        if let Err(message) = run(&disk, command, args) {
            println!("error: {message}");
        }
    }
    println!("files still open: {}", disk.open_count.get());
}
