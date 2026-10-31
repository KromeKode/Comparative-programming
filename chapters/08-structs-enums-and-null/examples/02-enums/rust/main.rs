#[derive(Clone, Copy, PartialEq)]
enum Light {
    Red,
    Green,
    Yellow,
}

impl Light {
    // Rust cannot list an enum's values for you, so the type keeps its own list.
    const ALL: [Light; 3] = [Light::Red, Light::Green, Light::Yellow];

    fn name(self) -> &'static str {
        match self {
            Light::Red => "red",
            Light::Green => "green",
            Light::Yellow => "yellow",
        }
    }

    fn next(self) -> Light {
        match self {
            Light::Red => Light::Green,
            Light::Green => Light::Yellow,
            Light::Yellow => Light::Red,
        }
    }

    fn meaning(self) -> &'static str {
        match self {
            Light::Red => "stop",
            Light::Green => "go",
            Light::Yellow => "slow down",
        }
    }

    fn seconds(self) -> u32 {
        match self {
            Light::Red => 30,
            Light::Green => 25,
            Light::Yellow => 5,
        }
    }

    fn parse(text: &str) -> Option<Light> {
        Light::ALL.into_iter().find(|light| light.name() == text)
    }
}

fn show_parse(text: &str) {
    match Light::parse(text) {
        Some(light) => println!("parse \"{text}\": {}", light.name()),
        None => println!("parse \"{text}\": not a light"),
    }
}

fn main() {
    let names: Vec<&str> = Light::ALL.iter().map(|light| light.name()).collect();
    println!("lights: {}", names.join(" "));
    println!("yellow is number {}", Light::Yellow as i32);

    let mut light = Light::Red;
    for _ in Light::ALL {
        println!("after {}: {}", light.name(), light.next().name());
        light = light.next();
    }
    println!(
        "red means {}, green means {}, yellow means {}",
        Light::Red.meaning(),
        Light::Green.meaning(),
        Light::Yellow.meaning()
    );

    let total: u32 = Light::ALL.iter().map(|light| light.seconds()).sum();
    println!("one cycle: {total} seconds");

    show_parse("green");
    show_parse("blue");
}
