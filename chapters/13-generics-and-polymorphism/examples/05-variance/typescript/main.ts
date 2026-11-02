interface Animal {
  readonly name: string;
}

// Without kind, a Dog would have the same shape as a Cat, and TypeScript
// would take either one where the other is expected.
class Cat implements Animal {
  readonly name: string;
  readonly kind = "cat";

  constructor(name: string) {
    this.name = name;
  }
}

class Dog implements Animal {
  readonly name: string;
  readonly kind = "dog";

  constructor(name: string) {
    this.name = name;
  }
}

// A readonly array only hands items out, so a Cat[] can be passed where a
// readonly Animal[] is expected.
function names(animals: readonly Animal[]): string {
  return animals.map((animal) => animal.name).join(", ");
}

const cats = [new Cat("Tom"), new Cat("Kitty")];
console.log(`names: ${names(cats)}`);

// TypeScript would also let a Cat[] be an Animal[] that takes dogs (section
// 13.7), so give a shelter that takes dogs its own array.
const shelter: Animal[] = [...cats, new Dog("Rex")];
console.log(`shelter: ${names(shelter)}`);

// A function that compares any two animals can compare two cats.
const byName = (a: Animal, b: Animal) =>
  a.name < b.name ? -1 : a.name > b.name ? 1 : 0;
console.log(`sorted by name: ${names(cats.toSorted(byName))}`);
