trait Animal {
    fn name(&self) -> &str;
}

#[derive(Clone)]
struct Cat(String);

impl Animal for Cat {
    fn name(&self) -> &str {
        &self.0
    }
}

struct Dog(String);

impl Animal for Dog {
    fn name(&self) -> &str {
        &self.0
    }
}

fn names(animals: &[&dyn Animal]) -> String {
    let texts: Vec<&str> = animals.iter().map(|animal| animal.name()).collect();
    texts.join(", ")
}

// A &Cat converts to a &dyn Animal, but a slice of Cats is not a slice of
// &dyn Animal: the items have different sizes. Make a new Vec.
fn as_animals(cats: &[Cat]) -> Vec<&dyn Animal> {
    cats.iter().map(|cat| cat as &dyn Animal).collect()
}

fn main() {
    let cats = vec![Cat("Tom".into()), Cat("Kitty".into())];
    let animals = as_animals(&cats);
    println!("names: {}", names(&animals));

    let rex = Dog("Rex".into());
    let mut shelter = animals.clone();
    shelter.push(&rex);
    println!("shelter: {}", names(&shelter));

    // A function for any two animals works on two cats, because each &Cat
    // converts as it is passed.
    let by_name = |a: &dyn Animal, b: &dyn Animal| a.name().cmp(b.name());
    let mut sorted = cats.clone();
    sorted.sort_by(|a, b| by_name(a, b));
    println!("sorted by name: {}", names(&as_animals(&sorted)));
}
