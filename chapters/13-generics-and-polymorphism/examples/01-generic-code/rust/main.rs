use std::fmt::Display;

// A generic type. T is a type parameter: each use of Stack fills it in.
struct Stack<T> {
    items: Vec<T>,
}

impl<T> Stack<T> {
    fn new() -> Self {
        Stack { items: Vec::new() }
    }

    fn push(&mut self, item: T) {
        self.items.push(item);
    }

    fn pop(&mut self) -> Option<T> {
        self.items.pop()
    }

    fn peek(&self) -> Option<&T> {
        self.items.last()
    }

    fn len(&self) -> usize {
        self.items.len()
    }
}

// A generic function. The bound T: PartialOrd lets the body use >, and only
// types that can be compared may be passed in.
fn largest<T: PartialOrd>(items: &[T]) -> Option<&T> {
    let mut best = items.first()?;
    for item in items {
        if item > best {
            best = item;
        }
    }
    Some(best)
}

// Two type parameters. swap returns a Pair with them the other way round.
struct Pair<A, B>(A, B);

impl<A, B> Pair<A, B> {
    fn swap(self) -> Pair<B, A> {
        Pair(self.1, self.0)
    }
}

// A Pair can be printed only if both of its parts can.
impl<A: Display, B: Display> Display for Pair<A, B> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "({}, {})", self.0, self.1)
    }
}

fn show<T: Display>(value: Option<T>) -> String {
    match value {
        Some(value) => value.to_string(),
        None => "none".to_string(),
    }
}

fn report<T: Display>(name: &str, stack: &mut Stack<T>) {
    let popped = show(stack.pop());
    let top = show(stack.peek());
    let left = stack.len();
    println!("{name}: popped {popped}, top is {top}, {left} left");
}

fn main() {
    let mut numbers = Stack::new(); // Stack<i32>, worked out from the pushes
    numbers.push(3);
    numbers.push(1);
    numbers.push(4);
    report("numbers", &mut numbers);

    let mut words: Stack<String> = Stack::new();
    words.push("pear".to_string());
    words.push("fig".to_string());
    report("words", &mut words);

    let mut empty: Stack<f64> = Stack::new(); // nothing to infer from, so say it
    report("empty", &mut empty);

    println!("largest number: {}", show(largest(&[3, 1, 4, 1, 5])));
    println!("largest word: {}", show(largest(&["pear", "fig", "apple"])));
    println!("largest of nothing: {}", show(largest::<i32>(&[])));

    let pair = Pair(7, "seven");
    let text = pair.to_string();
    println!("{text} swapped is {}", pair.swap());
}
