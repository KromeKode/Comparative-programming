trait Animal {
    fn name(&self) -> &str;
    fn sound(&self) -> &str;
}

struct Cat(String);

impl Cat {
    fn purr(&self) -> &str {
        "prrr"
    }
}

impl Animal for Cat {
    fn name(&self) -> &str {
        &self.0
    }

    fn sound(&self) -> &str {
        "meow"
    }
}

struct Dog(String);

impl Animal for Dog {
    fn name(&self) -> &str {
        &self.0
    }

    fn sound(&self) -> &str {
        "woof"
    }
}

// Static dispatch. The compiler makes one copy of chorus for Cat and another
// for Dog, and each copy calls that type's methods directly. All the animals
// in one call must be the same type.
fn chorus<A: Animal>(animals: &[A]) -> String {
    let lines: Vec<String> = animals
        .iter()
        .map(|animal| format!("{} says {}", animal.name(), animal.sound()))
        .collect();
    lines.join(", ")
}

// Dynamic dispatch. There is one copy, and each call looks up the method in
// the table that every &dyn Animal carries. The animals can be any mix.
fn chorus_dyn(animals: &[&dyn Animal]) -> String {
    let lines: Vec<String> = animals
        .iter()
        .map(|animal| format!("{} says {}", animal.name(), animal.sound()))
        .collect();
    lines.join(", ")
}

// A generic function hands back the caller's own type, so the caller can
// still use what only a Cat has.
fn longest_name<A: Animal>(animals: &[A]) -> &A {
    let mut best = &animals[0];
    for animal in animals {
        if animal.name().len() > best.name().len() {
            best = animal;
        }
    }
    best
}

fn main() {
    let cats = [Cat("Tom".into()), Cat("Kitty".into())];
    let dogs = [Dog("Rex".into())];
    println!("cats: {}", chorus(&cats));
    println!("dogs: {}", chorus(&dogs));

    let everyone: [&dyn Animal; 3] = [&cats[0], &dogs[0], &cats[1]];
    println!("everyone: {}", chorus_dyn(&everyone));

    let cat = longest_name(&cats);
    println!("longest name: {}, who purrs {}", cat.name(), cat.purr());
}
