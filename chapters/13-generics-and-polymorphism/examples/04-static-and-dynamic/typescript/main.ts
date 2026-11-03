interface Animal {
  readonly name: string;
  sound(): string;
}

class Cat implements Animal {
  readonly name: string;

  constructor(name: string) {
    this.name = name;
  }

  sound(): string {
    return "meow";
  }

  purr(): string {
    return "prrr";
  }
}

class Dog implements Animal {
  readonly name: string;

  constructor(name: string) {
    this.name = name;
  }

  sound(): string {
    return "woof";
  }
}

// The types are erased, so these two compile to the same JavaScript: one
// function that looks up sound() on each object as it runs.
function chorus<A extends Animal>(animals: readonly A[]): string {
  return animals
    .map((animal) => `${animal.name} says ${animal.sound()}`)
    .join(", ");
}

function chorusAll(animals: readonly Animal[]): string {
  return animals
    .map((animal) => `${animal.name} says ${animal.sound()}`)
    .join(", ");
}

// What the type parameter buys is the return type: the caller gets back a
// Cat, not just an Animal.
function longestName<A extends Animal>(animals: readonly A[]): A {
  let best = animals[0];
  for (const animal of animals) {
    if (animal.name.length > best.name.length) best = animal;
  }
  return best;
}

const cats = [new Cat("Tom"), new Cat("Kitty")];
const dogs = [new Dog("Rex")];
console.log(`cats: ${chorus(cats)}`);
console.log(`dogs: ${chorus(dogs)}`);

const everyone: Animal[] = [cats[0], dogs[0], cats[1]];
console.log(`everyone: ${chorusAll(everyone)}`);

const cat = longestName(cats);
console.log(`longest name: ${cat.name}, who purrs ${cat.purr()}`);
