// A bag of items. addAll is written in terms of add.
class Bag {
  #items = [];

  add(item) {
    this.#items.push(item);
  }

  addAll(items) {
    for (const item of items) {
      this.add(item); // calls the subclass's add, if it has one
    }
  }

  size() {
    return this.#items.length;
  }
}

// Inheritance: a CountingBag is a Bag that also counts what was added.
class InheritedCountingBag extends Bag {
  count = 0;

  add(item) {
    this.count++;
    super.add(item);
  }

  addAll(items) {
    this.count += items.length;
    super.addAll(items); // which calls this.add, which counts again
  }
}

// Composition: a CountingBag has a Bag, and passes the work on to it.
class ComposedCountingBag {
  #bag = new Bag();
  count = 0;

  add(item) {
    this.count++;
    this.#bag.add(item);
  }

  addAll(items) {
    this.count += items.length;
    this.#bag.addAll(items); // the inner Bag calls its own add
  }

  size() {
    return this.#bag.size();
  }
}

for (const [label, bag] of [
  ["inheritance", new InheritedCountingBag()],
  ["composition", new ComposedCountingBag()],
]) {
  bag.addAll(["apple", "pear", "plum"]);
  bag.add("fig");
  console.log(
    `${label}: the bag holds ${bag.size()} items, the count says ${bag.count}`,
  );
}
