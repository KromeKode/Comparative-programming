// One trait, implemented separately for each type. The compiler picks the
// implementation from the type, so each type gets its own code.
trait Show {
    fn show(&self) -> String;
}

impl Show for i32 {
    fn show(&self) -> String {
        self.to_string()
    }
}

impl Show for bool {
    fn show(&self) -> String {
        self.to_string()
    }
}

impl Show for &str {
    fn show(&self) -> String {
        format!("\"{self}\"")
    }
}

struct Point {
    x: i32,
    y: i32,
}

impl Show for Point {
    fn show(&self) -> String {
        format!("({}, {})", self.x, self.y)
    }
}

// One impl for every Vec whose items can be shown, so Vec<i32>,
// Vec<Vec<&str>>, and Vec<Point> all get show from these lines.
impl<T: Show> Show for Vec<T> {
    fn show(&self) -> String {
        let items: Vec<String> = self.iter().map(|item| item.show()).collect();
        format!("[{}]", items.join(", "))
    }
}

fn main() {
    println!("{}", 42.show());
    println!("{}", "hi".show());
    println!("{}", true.show());
    println!("{}", vec![1, 2, 3].show());
    println!("{}", vec![vec!["a", "b"], vec![], vec!["c"]].show());
    println!("{}", Point { x: 1, y: 2 }.show());
    let points = vec![Point { x: 1, y: 2 }, Point { x: 3, y: 4 }];
    println!("{}", points.show());
}
