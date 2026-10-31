use std::cell::RefCell;
use std::io::{self, BufRead};
use std::rc::{Rc, Weak};
use std::sync::atomic::{AtomicUsize, Ordering};

// Counts folders as they are dropped, so the program can report how many were
// freed. (An atomic is the simplest global counter Rust allows without unsafe;
// chapter 16 explains atomics.)
static FREED: AtomicUsize = AtomicUsize::new(0);

type Link = Rc<RefCell<Folder>>;

// A folder owns its children. Its link to its parent is Weak, so it does not
// own the parent. If it were an Rc, every parent and child would own each
// other, and no folder would ever be freed.
struct Folder {
    name: String,
    parent: Weak<RefCell<Folder>>,
    children: Vec<Link>,
}

impl Drop for Folder {
    fn drop(&mut self) {
        FREED.fetch_add(1, Ordering::Relaxed);
    }
}

fn freed() -> usize {
    FREED.load(Ordering::Relaxed)
}

fn folders(count: usize) -> String {
    if count == 1 {
        "1 folder".to_string()
    } else {
        format!("{count} folders")
    }
}

// Finds a folder by name, searching the whole tree from `from` down.
fn find(from: &Link, name: &str) -> Option<Link> {
    if from.borrow().name == name {
        return Some(Rc::clone(from));
    }
    from.borrow()
        .children
        .iter()
        .find_map(|child| find(child, name))
}

// Builds the path by following parent links up to the root.
fn path(folder: &Link) -> String {
    let mut names = Vec::new();
    let mut current = Some(Rc::clone(folder));
    while let Some(here) = current {
        let parent = here.borrow().parent.upgrade();
        if parent.is_some() {
            names.push(here.borrow().name.clone());
        }
        current = parent;
    }
    names.reverse();
    format!("/{}", names.join("/"))
}

// True when folder is inside ancestor, or is ancestor.
fn is_inside(folder: &Link, ancestor: &Link) -> bool {
    let mut current = Some(Rc::clone(folder));
    while let Some(here) = current {
        if Rc::ptr_eq(&here, ancestor) {
            return true;
        }
        current = here.borrow().parent.upgrade();
    }
    false
}

// Takes folder out of its parent's children. If nothing else owns it, the
// caller's Rc is now the only owner.
fn detach(folder: &Link) {
    if let Some(parent) = folder.borrow().parent.upgrade() {
        parent
            .borrow_mut()
            .children
            .retain(|c| !Rc::ptr_eq(c, folder));
    }
}

fn attach(folder: &Link, parent: &Link) {
    folder.borrow_mut().parent = Rc::downgrade(parent);
    parent.borrow_mut().children.push(Rc::clone(folder));
}

fn lookup(root: &Link, name: &str) -> Result<Link, String> {
    find(root, name).ok_or_else(|| format!("no folder \"{name}\""))
}

fn takes(command: &str, want: usize, got: usize) -> Result<(), String> {
    if got == want {
        return Ok(());
    }
    let noun = if want == 1 { "name" } else { "names" };
    Err(format!("{command} takes {want} {noun}, got {got}"))
}

fn run(root: &Link, command: &str, args: &[&str]) -> Result<(), String> {
    match command {
        "mkdir" => {
            takes(command, 2, args.len())?;
            if find(root, args[0]).is_some() {
                return Err(format!("\"{}\" already exists", args[0]));
            }
            let parent = lookup(root, args[1])?;
            let folder = Rc::new(RefCell::new(Folder {
                name: args[0].to_string(),
                parent: Weak::new(),
                children: Vec::new(),
            }));
            attach(&folder, &parent);
            println!("made {}", path(&folder));
        }
        "path" => {
            takes(command, 1, args.len())?;
            println!("{}", path(&lookup(root, args[0])?));
        }
        "ls" => {
            takes(command, 1, args.len())?;
            let folder = lookup(root, args[0])?;
            let folder = folder.borrow();
            let names: Vec<String> = folder
                .children
                .iter()
                .map(|c| c.borrow().name.clone())
                .collect();
            let list = if names.is_empty() {
                "(empty)".to_string()
            } else {
                names.join(" ")
            };
            println!("{}: {list}", folder.name);
        }
        "rm" => {
            takes(command, 1, args.len())?;
            let folder = lookup(root, args[0])?;
            if Rc::ptr_eq(&folder, root) {
                return Err("cannot remove /".to_string());
            }
            let before = freed();
            detach(&folder);
            drop(folder); // the last owner: the folder and its children go
            let count = freed() - before;
            println!("removed {}, freed {}", args[0], folders(count));
        }
        "mv" => {
            takes(command, 2, args.len())?;
            let folder = lookup(root, args[0])?;
            if Rc::ptr_eq(&folder, root) {
                return Err("cannot move /".to_string());
            }
            let parent = lookup(root, args[1])?;
            if is_inside(&parent, &folder) {
                return Err(format!("cannot move {} into itself", args[0]));
            }
            detach(&folder);
            attach(&folder, &parent);
            println!("moved {} to {}", args[0], path(&folder));
        }
        _ => return Err(format!("unknown command \"{command}\"")),
    }
    Ok(())
}

fn main() {
    let root = Rc::new(RefCell::new(Folder {
        name: "/".to_string(),
        parent: Weak::new(),
        children: Vec::new(),
    }));
    for line in io::stdin().lock().lines() {
        let line = line.expect("failed to read stdin");
        let parts: Vec<&str> = line.split_whitespace().collect();
        let Some((command, args)) = parts.split_first() else {
            continue;
        };
        if let Err(message) = run(&root, command, args) {
            println!("error: {message}");
        }
    }
    let before = freed();
    drop(root);
    println!("freed {} at exit", folders(freed() - before));
}
