use std::fmt::Display;
use std::ops::Add;

// Money keeps whole cents, so adding never rounds.
#[derive(Clone, Copy, Default, PartialEq, PartialOrd)]
struct Money(i64);

// Implementing Add is what lets Money use +, and what lets it meet total's bound.
impl Add for Money {
    type Output = Money;

    fn add(self, other: Money) -> Money {
        Money(self.0 + other.0)
    }
}

impl Display for Money {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "${}.{:02}", self.0 / 100, self.0 % 100)
    }
}

// The bounds say what total needs from T: a + that gives back a T, a
// default value to start from, and copying.
fn total<T: Add<Output = T> + Default + Copy>(items: &[T]) -> T {
    let mut sum = T::default();
    for &item in items {
        sum = sum + item;
    }
    sum
}

// When there are several bounds, a where clause keeps the signature readable.
fn describe_range<T>(items: &[T]) -> String
where
    T: PartialOrd + Display,
{
    let (mut low, mut high) = (&items[0], &items[0]);
    for item in items {
        if item < low {
            low = item;
        }
        if item > high {
            high = item;
        }
    }
    format!("{low} to {high}")
}

fn list<T: Display>(items: &[T]) -> String {
    let texts: Vec<String> = items.iter().map(|item| item.to_string()).collect();
    texts.join(", ")
}

fn main() {
    let numbers = [1, 2, 3];
    let halves = [0.5, 0.25];
    let prices = [Money(150), Money(225)];
    println!("total of {}: {}", list(&numbers), total(&numbers));
    println!("total of {}: {}", list(&halves), total(&halves));
    println!("total of {}: {}", list(&prices), total(&prices));

    let digits = [3, 9, 1];
    let fruit = ["pear", "fig", "apple"];
    let costs = [Money(200), Money(99)];
    println!("range of {}: {}", list(&digits), describe_range(&digits));
    println!("range of {}: {}", list(&fruit), describe_range(&fruit));
    println!("range of {}: {}", list(&costs), describe_range(&costs));
}
