use std::io::{self, BufRead};

type Grid = Vec<Vec<char>>;

// Reads a number made of digits. One too big for usize becomes usize::MAX,
// which is outside any grid.
fn parse_number(text: &str) -> Option<usize> {
    if !text.bytes().all(|b| b.is_ascii_digit()) {
        println!("error: \"{text}\" is not a number");
        return None;
    }
    Some(text.parse().unwrap_or(usize::MAX))
}

fn one_character(text: &str) -> Option<char> {
    let mut chars = text.chars();
    match (chars.next(), chars.next()) {
        (Some(c), None) => Some(c),
        _ => {
            println!("error: \"{text}\" is not one character");
            None
        }
    }
}

fn takes(command: &str, want: usize, got: usize) -> bool {
    if got != want {
        let plural = if want == 1 { "" } else { "s" };
        println!("error: {command} takes {want} value{plural}, got {got}");
    }
    got == want
}

fn main() {
    let mut lines = io::stdin()
        .lock()
        .lines()
        .map(|line| line.expect("failed to read stdin"));
    let Some(first) = lines.next() else {
        return;
    };
    let sizes: Vec<usize> = first
        .split_whitespace()
        .filter_map(|s| s.parse().ok())
        .collect();
    let (width, height) = (sizes[0], sizes[1]);
    // vec![row; height] clones the row for each slot, so the rows are separate.
    let mut grid: Grid = vec![vec!['.'; width]; height];
    let mut saved: Option<Grid> = None;

    for line in lines {
        let parts: Vec<&str> = line.split_whitespace().collect();
        let Some((&command, values)) = parts.split_first() else {
            continue;
        };
        match command {
            "paint" => {
                if !takes(command, 3, values.len()) {
                    continue;
                }
                let Some(x) = parse_number(values[0]) else {
                    continue;
                };
                let Some(y) = parse_number(values[1]) else {
                    continue;
                };
                if !(1..=width).contains(&x) || !(1..=height).contains(&y) {
                    let (tx, ty) = (values[0], values[1]);
                    println!("error: ({tx}, {ty}) is outside the {width} by {height} grid");
                    continue;
                }
                if let Some(c) = one_character(values[2]) {
                    grid[y - 1][x - 1] = c;
                }
            }
            "fill" => {
                if !takes(command, 1, values.len()) {
                    continue;
                }
                if let Some(c) = one_character(values[0]) {
                    for row in grid.iter_mut() {
                        row.fill(c);
                    }
                }
            }
            // `saved = Some(grid)` would move the grid into saved and leave no
            // grid to paint on, so the compiler makes us clone.
            "save" => {
                if takes(command, 0, values.len()) {
                    saved = Some(grid.clone());
                }
            }
            "restore" => {
                if !takes(command, 0, values.len()) {
                    continue;
                }
                match &saved {
                    Some(snapshot) => grid = snapshot.clone(),
                    None => println!("error: nothing saved"),
                }
            }
            "show" => {
                if takes(command, 0, values.len()) {
                    for row in &grid {
                        println!("{}", row.iter().collect::<String>());
                    }
                    println!();
                }
            }
            _ => println!("error: unknown command \"{command}\""),
        }
    }
}
