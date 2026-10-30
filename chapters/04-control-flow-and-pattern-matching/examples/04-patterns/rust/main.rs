fn classify(point: (i32, i32)) -> String {
    match point {
        (0, 0) => "origin".to_string(),
        (x, 0) => format!("on the x axis at x = {x}"), // x binds the first value
        (0, y) => format!("on the y axis at y = {y}"),
        (x, y) if x == y => format!("on the diagonal at {x}"), // a guard
        (x, y) => format!("x = {x}, y = {y}"),
    }
}

fn main() {
    for (x, y) in [(0, 0), (3, 0), (0, -2), (4, 4), (-1, 5)] {
        println!("({x}, {y}): {}", classify((x, y)));
    }
}
