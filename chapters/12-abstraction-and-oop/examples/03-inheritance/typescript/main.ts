// A bag of items. addAll is written in terms of add.
class Bag {
  private readonly items: string[] = [];

  add(item: string): void {
    this.items.push(item);
  }

  addAll(items: string[]): void {
    for (const item of items) {
      this.add(item); // calls the subclass's add, if it has one
    }
  }

  size(): number {
    return this.items.length;
  }
}

// Inheritance: a CountingBag is a Bag that also counts what was added.
class InheritedCountingBag extends Bag {
  count = 0;

  override add(item: string): void {
    this.count++;
    super.add(item);
  }

  override addAll(items: string[]): void {
    this.count += items.length;
    super.addAll(items); // which calls this.add, which counts again
  }
}

// Composition: a CountingBag has a Bag, and passes the work on to it.
class ComposedCountingBag {
  private readonly bag = new Bag();
  count = 0;

  add(item: string): void {
    this.count++;
    this.bag.add(item);
  }

  addAll(items: string[]): void {
    this.count += items.length;
    this.bag.addAll(items); // the inner Bag calls its own add
  }

  size(): number {
    return this.bag.size();
  }
}

const bags: [string, InheritedCountingBag | ComposedCountingBag][] = [
  ["inheritance", new InheritedCountingBag()],
  ["composition", new ComposedCountingBag()],
];
for (const [label, bag] of bags) {
  bag.addAll(["apple", "pear", "plum"]);
  bag.add("fig");
  console.log(
    `${label}: the bag holds ${bag.size()} items, the count says ${bag.count}`,
  );
}
