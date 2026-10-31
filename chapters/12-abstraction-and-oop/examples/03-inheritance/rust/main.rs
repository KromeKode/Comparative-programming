// Rust has no inheritance. A trait can still share code: add_all has a default
// body, written in terms of add, which every type that implements the trait
// must provide.
trait Collection {
    fn add(&mut self, item: &str);
    fn size(&self) -> usize;

    fn add_all(&mut self, items: &[&str]) {
        for item in items {
            self.add(item);
        }
    }
}

#[derive(Default)]
struct Bag {
    items: Vec<String>,
}

impl Collection for Bag {
    fn add(&mut self, item: &str) {
        self.items.push(item.to_string());
    }

    fn size(&self) -> usize {
        self.items.len()
    }
}

// Composition: a CountingBag has a Bag, and passes the work on to it.
#[derive(Default)]
struct CountingBag {
    bag: Bag,
    count: usize,
}

impl Collection for CountingBag {
    fn add(&mut self, item: &str) {
        self.count += 1;
        self.bag.add(item);
    }

    fn size(&self) -> usize {
        self.bag.size()
    }

    // add_all is not written here, so the default one is used. It calls
    // CountingBag's add, which counts each item once.
}

fn main() {
    let mut bag = CountingBag::default();
    bag.add_all(&["apple", "pear", "plum"]);
    bag.add("fig");
    println!(
        "composition: the bag holds {} items, the count says {}",
        bag.size(),
        bag.count
    );
}
