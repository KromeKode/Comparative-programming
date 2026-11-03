// A ring keeps the last three items pushed into it, oldest first.
class Ring implements Iterable<number> {
  #items = [0, 0, 0];
  #start = 0;
  #len = 0;

  push(item: number): void {
    this.#items[(this.#start + this.#len) % 3] = item;
    if (this.#len < 3) {
      this.#len++;
    } else {
      this.#start = (this.#start + 1) % 3; // the oldest item is overwritten
    }
  }

  // for...of calls the method named Symbol.iterator to get an iterator: an
  // object whose next() returns { value, done }.
  [Symbol.iterator](): IterableIterator<number> {
    const items = this.#items;
    const start = this.#start;
    const len = this.#len;
    let index = 0;
    return {
      next(): IteratorResult<number> {
        if (index === len) return { value: undefined, done: true };
        return { value: items[(start + index++) % 3], done: false };
      },
      // An iterator is iterable too: it returns itself, so a loop over it
      // carries on from where it is.
      [Symbol.iterator]() {
        return this;
      },
    };
  }
}

const ring = new Ring();
for (let i = 1; i <= 5; i++) ring.push(i);

let line = "loop:";
for (const item of ring) line += ` ${item}`;
console.log(line);
console.log(`again: ${[...ring].join(" ")}`); // spreading asks for a new iterator too

let it = ring[Symbol.iterator]();
const parts: number[] = [];
for (let step = it.next(); !step.done; step = it.next()) parts.push(step.value);
console.log(`by hand: ${parts.join(", then ")}, then nothing`);

it = ring[Symbol.iterator]();
const first = it.next().value;
line = `resumed: took ${first}, then the loop got`;
for (const item of it) line += ` ${item}`; // carries on from where next left off
console.log(line);
