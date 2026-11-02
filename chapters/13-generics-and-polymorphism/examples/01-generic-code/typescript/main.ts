// A generic class. T is a type parameter: each use of Stack fills it in.
class Stack<T> {
  #items: T[] = [];

  push(item: T): void {
    this.#items.push(item);
  }

  pop(): T | undefined {
    return this.#items.pop();
  }

  peek(): T | undefined {
    return this.#items.at(-1);
  }

  get size(): number {
    return this.#items.length;
  }
}

// A generic function. TypeScript has no "can be ordered" constraint, so this
// one allows numbers and strings, the types that > compares sensibly.
function largest<T extends number | string>(
  items: readonly T[],
): T | undefined {
  let best = items[0];
  for (const item of items) {
    if (best === undefined || item > best) best = item;
  }
  return best;
}

// Two type parameters. swap returns a Pair with them the other way round.
class Pair<A, B> {
  readonly first: A;
  readonly second: B;

  constructor(first: A, second: B) {
    this.first = first;
    this.second = second;
  }

  swap(): Pair<B, A> {
    return new Pair(this.second, this.first);
  }

  toString(): string {
    return `(${this.first}, ${this.second})`;
  }
}

function show<T>(value: T | undefined): string {
  return value === undefined ? "none" : String(value);
}

function report<T>(name: string, stack: Stack<T>): void {
  const popped = show(stack.pop());
  const top = show(stack.peek());
  console.log(`${name}: popped ${popped}, top is ${top}, ${stack.size} left`);
}

const numbers = new Stack<number>();
numbers.push(3);
numbers.push(1);
numbers.push(4);
report("numbers", numbers);

const words = new Stack<string>();
words.push("pear");
words.push("fig");
report("words", words);

const empty = new Stack<number>();
report("empty", empty);

console.log(`largest number: ${show(largest([3, 1, 4, 1, 5]))}`);
console.log(`largest word: ${show(largest(["pear", "fig", "apple"]))}`);
console.log(`largest of nothing: ${show(largest<number>([]))}`);

const pair = new Pair(7, "seven"); // Pair<number, string>, inferred
console.log(`${pair} swapped is ${pair.swap()}`);
