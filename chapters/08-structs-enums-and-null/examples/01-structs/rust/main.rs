use std::fmt;

// derive asks the compiler to write clone, copy, and == for Point.
#[derive(Clone, Copy, PartialEq)]
struct Point {
    x: i32,
    y: i32,
}

// Methods live in an impl block, apart from the fields.
impl Point {
    fn steps_from_origin(&self) -> i32 {
        self.x.abs() + self.y.abs()
    }
}

// Display is how {} prints a Point.
impl fmt::Display for Point {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "({}, {})", self.x, self.y)
    }
}

fn main() {
    let p = Point { x: 1, y: 2 };
    println!("p = {p}");
    println!("p.x + p.y = {}", p.x + p.y);

    // Struct update syntax: x is 5, and the other fields come from p.
    let q = Point { x: 5, ..p };
    println!("q = p with x = 5: q = {q}, p = {p}");

    // Point is Copy, so this copies. Without Copy, it would move p into r.
    let mut r = p;
    r.y = 9;
    println!("r = p, then r.y = 9: r = {r}, p = {p}");

    let a = Point { x: 3, y: -4 };
    let b = Point { x: 3, y: -4 };
    let same = if a == b { "yes" } else { "no" };
    println!("a = {a}, b = {b}, a == b: {same}");
    println!("steps from (0, 0) to a: {}", a.steps_from_origin());
}
