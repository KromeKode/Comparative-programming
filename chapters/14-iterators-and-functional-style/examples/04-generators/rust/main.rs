// Rust has no generators yet (gen blocks are unstable). from_fn makes an
// iterator from a closure that keeps its own state between calls.
fn fibonacci() -> impl Iterator<Item = u64> {
    let (mut a, mut b) = (0, 1);
    std::iter::from_fn(move || {
        let value = a;
        (a, b) = (b, a + b);
        Some(value)
    })
}

struct Node {
    value: i32,
    left: Option<Box<Node>>,
    right: Option<Box<Node>>,
}

fn node(value: i32, left: Option<Box<Node>>, right: Option<Box<Node>>) -> Option<Box<Node>> {
    Some(Box::new(Node { value, left, right }))
}

fn leaf(value: i32) -> Option<Box<Node>> {
    node(value, None, None)
}

// Without a generator to keep the path down the tree, the iterator keeps it
// itself: a stack of the nodes whose value hasn't been handed out yet.
struct InOrder<'a> {
    stack: Vec<&'a Node>,
}

impl<'a> InOrder<'a> {
    fn new(root: &'a Option<Box<Node>>) -> Self {
        let mut it = InOrder { stack: Vec::new() };
        it.push_left(root.as_deref());
        it
    }

    fn push_left(&mut self, mut node: Option<&'a Node>) {
        while let Some(n) = node {
            self.stack.push(n);
            node = n.left.as_deref();
        }
    }
}

impl Iterator for InOrder<'_> {
    type Item = i32;

    fn next(&mut self) -> Option<i32> {
        let node = self.stack.pop()?;
        self.push_left(node.right.as_deref());
        Some(node.value)
    }
}

fn line(items: impl Iterator<Item = impl ToString>) -> String {
    items
        .map(|item| item.to_string())
        .collect::<Vec<_>>()
        .join(" ")
}

fn main() {
    let tree = node(4, node(2, leaf(1), leaf(3)), node(6, leaf(5), leaf(7)));
    println!("fibonacci: {}", line(fibonacci().take(10)));
    println!("in order: {}", line(InOrder::new(&tree)));
    // take_while stops asking at the first value over 3, so 5, 6, and 7 are
    // never visited.
    println!(
        "up to 3: {}",
        line(InOrder::new(&tree).take_while(|&v| v <= 3))
    );
}
