// A ring keeps the last three items pushed into it, oldest first.
struct Ring {
    items: [i32; 3],
    start: usize,
    len: usize,
}

impl Ring {
    fn new() -> Self {
        Ring {
            items: [0; 3],
            start: 0,
            len: 0,
        }
    }

    fn push(&mut self, item: i32) {
        self.items[(self.start + self.len) % 3] = item;
        if self.len < 3 {
            self.len += 1;
        } else {
            self.start = (self.start + 1) % 3; // the oldest item is overwritten
        }
    }

    fn iter(&self) -> RingIter<'_> {
        RingIter {
            ring: self,
            index: 0,
        }
    }
}

// The iterator borrows the ring, so the ring can't change while it is in use.
struct RingIter<'a> {
    ring: &'a Ring,
    index: usize,
}

// One required method: next returns Some(item), or None at the end. Every
// other Iterator method, such as map, filter, and sum, comes with it.
impl Iterator for RingIter<'_> {
    type Item = i32;

    fn next(&mut self) -> Option<i32> {
        if self.index == self.ring.len {
            return None;
        }
        let item = self.ring.items[(self.ring.start + self.index) % 3];
        self.index += 1;
        Some(item)
    }
}

// IntoIterator is what a for loop calls, so this makes `for item in &ring` work.
impl<'a> IntoIterator for &'a Ring {
    type Item = i32;
    type IntoIter = RingIter<'a>;

    fn into_iter(self) -> RingIter<'a> {
        self.iter()
    }
}

fn main() {
    let mut ring = Ring::new();
    for i in 1..=5 {
        ring.push(i);
    }

    print!("loop:");
    for item in &ring {
        print!(" {item}");
    }
    let again: Vec<String> = ring.iter().map(|item| item.to_string()).collect();
    println!("\nagain: {}", again.join(" ")); // each iter() starts at the beginning

    let mut it = ring.iter();
    print!("by hand:");
    let mut sep = " ";
    while let Some(item) = it.next() {
        print!("{sep}{item}");
        sep = ", then ";
    }
    println!(", then nothing");

    let mut it = ring.iter();
    let first = it.next().unwrap();
    print!("resumed: took {first}, then the loop got");
    for item in it {
        print!(" {item}"); // the loop carries on from where next left off
    }
    println!();
}
