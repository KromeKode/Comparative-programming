use std::cell::RefCell;
use std::rc::{Rc, Weak};

// Rc counts owners. RefCell lets shared nodes change their links.
struct Node {
    name: &'static str,
    next: Option<Rc<RefCell<Node>>>, // owns what it points to
    back: Weak<RefCell<Node>>,       // does not
}

fn node(name: &'static str) -> Node {
    println!("  make {name}");
    Node {
        name,
        next: None,
        back: Weak::new(),
    }
}

fn make(name: &'static str) -> Rc<RefCell<Node>> {
    Rc::new(RefCell::new(node(name)))
}

impl Drop for Node {
    fn drop(&mut self) {
        println!("  drop {}", self.name);
    }
}

fn yes_no(flag: bool) -> &'static str {
    if flag { "yes" } else { "no" }
}

fn main() {
    println!("one owner:");
    {
        let _report = Box::new(node("report"));
    }

    println!("two owners:");
    {
        let doc = make("doc");
        let also = Rc::clone(&doc);
        println!("  owners: {}", Rc::strong_count(&doc));
        drop(doc);
        println!("  one owner let go, owners: {}", Rc::strong_count(&also));
    }

    println!("a weak pointer does not own:");
    let weak;
    {
        let cache = make("cache");
        weak = Rc::downgrade(&cache);
        let reach = weak.upgrade().is_some();
        println!("  weak can reach cache: {}", yes_no(reach));
    }
    let reach = weak.upgrade().is_some();
    println!("  weak can reach cache: {}", yes_no(reach));

    println!("a cycle of owning links:");
    let watch;
    {
        let a = make("a");
        let b = make("b");
        a.borrow_mut().next = Some(Rc::clone(&b));
        b.borrow_mut().next = Some(Rc::clone(&a));
        watch = Rc::downgrade(&a);
    }
    let owners = watch.strong_count();
    println!("  a and b went out of scope, and a still has {owners} owner");
    println!("  break the cycle by hand:");
    if let Some(a) = watch.upgrade() {
        a.borrow_mut().next = None;
    }

    println!("an owning link one way, a weak link back:");
    {
        let a = make("a");
        let b = make("b");
        a.borrow_mut().next = Some(Rc::clone(&b));
        b.borrow_mut().back = Rc::downgrade(&a);
        if let Some(back) = b.borrow().back.upgrade() {
            println!("  b's back link reaches {}", back.borrow().name);
        }
    }
}
