// JavaScript has no types to fill in, so every class and function already
// works with any type. This is the same code as the TypeScript version,
// without the type parameters.
class Stack {
  #items = [];

  push(item) {
    this.#items.push(item);
  }

  pop() {
    return this.#items.pop();
  }

  peek() {
    return this.#items.at(-1);
  }

  get size() {
    return this.#items.length;
  }
}

// Works on anything > works on. On objects it gives nonsense, not an error.
function largest(items) {
  let best = items[0];
  for (const item of items) {
    if (best === undefined || item > best) best = item;
  }
  return best;
}

class Pair {
  constructor(first, second) {
    this.first = first;
    this.second = second;
  }

  swap() {
    return new Pair(this.second, this.first);
  }

  toString() {
    return `(${this.first}, ${this.second})`;
  }
}

function show(value) {
  return value === undefined ? "none" : String(value);
}

function report(name, stack) {
  const popped = show(stack.pop());
  const top = show(stack.peek());
  console.log(`${name}: popped ${popped}, top is ${top}, ${stack.size} left`);
}

const numbers = new Stack();
numbers.push(3);
numbers.push(1);
numbers.push(4);
report("numbers", numbers);

const words = new Stack();
words.push("pear");
words.push("fig");
report("words", words);

report("empty", new Stack());

console.log(`largest number: ${show(largest([3, 1, 4, 1, 5]))}`);
console.log(`largest word: ${show(largest(["pear", "fig", "apple"]))}`);
console.log(`largest of nothing: ${show(largest([]))}`);

const pair = new Pair(7, "seven");
console.log(`${pair} swapped is ${pair.swap()}`);
