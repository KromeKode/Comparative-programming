// A trait can give a method a default body. Every type that implements
// Describe must say what its name is, and gets describe for free.
trait Describe {
    fn name(&self) -> &str;

    fn describe(&self) -> String {
        format!("this is the {}", self.name())
    }
}

// Logged needs a name too, so it requires Describe (a supertrait). A trait has
// no fields, so it asks the type for access to the history the type stores.
trait Logged: Describe {
    fn history(&self) -> &[String];
    fn history_mut(&mut self) -> &mut Vec<String>;

    fn log(&mut self, event: &str) {
        let line = format!("{}: {event}", self.name());
        self.history_mut().push(line);
    }
}

struct Door {
    name: String,
    history: Vec<String>,
}

impl Door {
    fn open(&mut self) {
        self.log("opened");
    }

    fn close(&mut self) {
        self.log("closed");
    }
}

impl Describe for Door {
    fn name(&self) -> &str {
        &self.name
    }
}

impl Logged for Door {
    fn history(&self) -> &[String] {
        &self.history
    }

    fn history_mut(&mut self) -> &mut Vec<String> {
        &mut self.history
    }
}

struct Light {
    name: String,
    history: Vec<String>,
    on: bool,
}

impl Light {
    fn switch_on(&mut self) {
        self.on = true;
        self.log("switched on");
    }
}

// Light replaces the default describe. Rust has no way to call the default
// body from here, so the text is written out again.
impl Describe for Light {
    fn name(&self) -> &str {
        &self.name
    }

    fn describe(&self) -> String {
        let state = if self.on { "on" } else { "off" };
        format!("this is the {}, and it is {state}", self.name)
    }
}

impl Logged for Light {
    fn history(&self) -> &[String] {
        &self.history
    }

    fn history_mut(&mut self) -> &mut Vec<String> {
        &mut self.history
    }
}

// This function needs only Describe.
fn print_description(thing: &dyn Describe) {
    println!("{}", thing.describe());
}

fn main() {
    let mut door = Door {
        name: "front door".to_string(),
        history: Vec::new(),
    };
    let mut light = Light {
        name: "porch light".to_string(),
        history: Vec::new(),
        on: false,
    };
    door.open();
    door.close();
    light.switch_on();

    let things: [&dyn Logged; 2] = [&door, &light];
    for thing in things {
        // A &dyn Logged converts to a &dyn Describe, because Describe is a
        // supertrait of Logged (trait upcasting, Rust 1.86 and later).
        print_description(thing);
        for line in thing.history() {
            println!("  {line}");
        }
    }
}
