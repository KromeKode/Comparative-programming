use std::io::{self, BufRead};

#[derive(Clone, Copy)]
enum Direction {
    North,
    East,
    South,
    West,
}

impl Direction {
    fn right(self) -> Direction {
        match self {
            Direction::North => Direction::East,
            Direction::East => Direction::South,
            Direction::South => Direction::West,
            Direction::West => Direction::North,
        }
    }

    fn left(self) -> Direction {
        self.right().right().right()
    }

    // How far one step forward moves in x and y.
    fn step(self) -> (i64, i64) {
        match self {
            Direction::North => (0, 1),
            Direction::East => (1, 0),
            Direction::South => (0, -1),
            Direction::West => (-1, 0),
        }
    }

    fn name(self) -> &'static str {
        match self {
            Direction::North => "north",
            Direction::East => "east",
            Direction::South => "south",
            Direction::West => "west",
        }
    }
}

struct Robot {
    x: i64,
    y: i64,
    facing: Direction,
}

impl Robot {
    fn new() -> Robot {
        Robot {
            x: 0,
            y: 0,
            facing: Direction::North,
        }
    }

    fn run(&mut self, command: char) {
        match command {
            'L' => self.facing = self.facing.left(),
            'R' => self.facing = self.facing.right(),
            'F' => {
                let (dx, dy) = self.facing.step();
                self.x += dx;
                self.y += dy;
            }
            _ => unreachable!("commands are checked before they run"),
        }
    }
}

fn main() {
    for line in io::stdin().lock().lines() {
        let line = line.expect("failed to read stdin");
        if line.is_empty() {
            continue;
        }
        let bad = line.chars().zip(1..).find(|(c, _)| !"LRF".contains(*c));
        if let Some((command, position)) = bad {
            println!("error: unknown command '{command}' at position {position}");
            continue;
        }
        let mut robot = Robot::new();
        for command in line.chars() {
            robot.run(command);
        }
        println!("{} {} {}", robot.x, robot.y, robot.facing.name());
    }
}
