use std::f64::consts::PI;
use std::io::{self, BufRead};

enum Shape {
    Circle { radius: f64 },
    Rectangle { width: f64, height: f64 },
    Triangle { base: f64, height: f64 },
}

impl Shape {
    fn area(&self) -> f64 {
        match self {
            Shape::Circle { radius } => PI * radius * radius,
            Shape::Rectangle { width, height } => width * height,
            Shape::Triangle { base, height } => base * height / 2.0,
        }
    }
}

fn parse_size(text: &str) -> Result<f64, String> {
    let value: f64 = text
        .parse()
        .map_err(|_| format!("\"{text}\" is not a number"))?;
    if value < 0.0 {
        return Err(format!("\"{text}\" is negative"));
    }
    Ok(value)
}

// Turns a shape name and its sizes into a Shape, or an error message.
fn parse(name: &str, texts: &[&str]) -> Result<Shape, String> {
    let needed = match name {
        "circle" => 1,
        "rectangle" | "triangle" => 2,
        _ => return Err(format!("unknown shape \"{name}\"")),
    };
    if texts.len() != needed {
        let plural = if needed == 1 { "" } else { "s" };
        return Err(format!(
            "{name} needs {needed} size{plural}, got {}",
            texts.len()
        ));
    }
    let sizes = texts
        .iter()
        .map(|text| parse_size(text))
        .collect::<Result<Vec<f64>, String>>()?;
    Ok(match name {
        "circle" => Shape::Circle { radius: sizes[0] },
        "rectangle" => Shape::Rectangle {
            width: sizes[0],
            height: sizes[1],
        },
        _ => Shape::Triangle {
            base: sizes[0],
            height: sizes[1],
        },
    })
}

fn main() {
    let mut total = 0.0;
    for line in io::stdin().lock().lines() {
        let line = line.expect("failed to read stdin");
        let words: Vec<&str> = line.split_whitespace().collect();
        let Some((name, texts)) = words.split_first() else {
            continue; // a blank line
        };
        match parse(name, texts) {
            Ok(shape) => {
                total += shape.area();
                println!("{name}: {:.2}", shape.area());
            }
            Err(message) => println!("error: {message}"),
        }
    }
    println!("total: {total:.2}");
}
