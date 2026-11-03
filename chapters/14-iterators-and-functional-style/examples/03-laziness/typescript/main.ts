// Both functions say when they run, so the output shows the order of the work.
function square(n: number): number {
  console.log(`  square ${n}`);
  return n * n;
}

function even(x: number): boolean {
  console.log(`  even? ${x}`);
  return x % 2 === 0;
}

const numbers = [1, 2, 3, 4, 5];

// Eager: each array method builds a whole new array before the next one runs.
console.log("eager:");
const evens = numbers.map(square).filter(even);
console.log(`  first two: ${evens.slice(0, 2).join(" ")}`);

// Lazy: values() gives an iterator, and iterator helpers (ES2025) such as
// map, filter, and take pull one item at a time through every step.
console.log("lazy:");
const firstTwo = numbers.values().map(square).filter(even).take(2).toArray();
console.log(`  first two: ${firstTwo.join(" ")}`);

// A generator that counts up forever, so only a lazy pipeline can use it.
function* naturals(): Generator<number> {
  for (let n = 1; ; n++) yield n;
}

const endless = naturals()
  .map((n) => n * n)
  .filter((x) => x % 2 === 0 && x > 50)
  .take(3)
  .toArray();
console.log(`endless: ${endless.join(" ")}`);
