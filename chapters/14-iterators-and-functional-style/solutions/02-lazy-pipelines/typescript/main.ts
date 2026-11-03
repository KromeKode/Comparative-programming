import { readFileSync } from "node:fs";

// IteratorObject is the type of the built-in iterators, generators included,
// which all have the helper methods.
type Items = IteratorObject<number>;

function isPrime(n: number): boolean {
  if (n < 2) return false;
  for (let d = 2; d * d <= n; d++) if (n % d === 0) return false;
  return true;
}

// The sources are generators, so the endless ones cost nothing until asked.
function* naturals(): Generator<number> {
  for (let n = 1; ; n++) yield n;
}

function* fibonacci(): Generator<number> {
  for (let [a, b] = [0, 1]; ; [a, b] = [b, a + b]) yield a;
}

function* range(low: number, high: number): Generator<number> {
  for (let n = low; n <= high; n++) yield n;
}

function* repeat(n: number): Generator<number> {
  for (;;) yield n;
}

// The iterator helpers have map, filter, take, and drop, but no takeWhile,
// dropWhile, or step, so those three are generators too.
function* takeWhile(
  items: Items,
  keep: (x: number) => boolean,
): Generator<number> {
  for (const item of items) {
    if (!keep(item)) return;
    yield item;
  }
}

function* dropWhile(
  items: Items,
  drop: (x: number) => boolean,
): Generator<number> {
  let dropping = true;
  for (const item of items) {
    if (dropping && drop(item)) continue;
    dropping = false;
    yield item;
  }
}

function* step(items: Items, n: number): Generator<number> {
  let i = 0;
  for (const item of items) {
    if (i++ % n === 0) yield item;
  }
}

function source(words: string[]): Items {
  const [name, a, b] = words;
  switch (name) {
    case "naturals":
      return naturals();
    case "primes":
      return naturals().filter(isPrime);
    case "fibonacci":
      return fibonacci();
    case "range":
      return range(Number(a), Number(b));
    case "repeat":
      return repeat(Number(a));
  }
  throw new Error(`unknown source ${name}`);
}

function predicate(words: string[]): (x: number) => boolean {
  const [name, n] = words;
  switch (name) {
    case "even":
      return (x) => x % 2 === 0;
    case "odd":
      return (x) => x % 2 !== 0;
    case "prime":
      return isPrime;
    case "above":
      return (x) => x > Number(n);
    case "below":
      return (x) => x < Number(n);
  }
  throw new Error(`unknown predicate ${name}`);
}

function func(words: string[]): (x: number) => number {
  const [name, n] = words;
  switch (name) {
    case "square":
      return (x) => x * x;
    case "negate":
      return (x) => -x;
    case "add":
      return (x) => x + Number(n);
    case "mul":
      return (x) => x * Number(n);
  }
  throw new Error(`unknown function ${name}`);
}

// Each stage wraps the iterator it is given in a new lazy one.
function stage(items: Items, words: string[]): Items {
  const [name, ...rest] = words;
  const n = Number(rest[0]);
  switch (name) {
    case "map":
      return items.map(func(rest));
    case "filter":
      return items.filter(predicate(rest));
    case "takewhile":
      return takeWhile(items, predicate(rest));
    case "dropwhile":
      return dropWhile(items, predicate(rest));
    case "take":
      return items.take(n);
    case "skip":
      return items.drop(n);
    case "step":
      return step(items, n);
  }
  throw new Error(`unknown stage ${name}`);
}

function run(line: string): string {
  const parts = line
    .split("|")
    .map((part) => part.split(/\s+/).filter(Boolean));
  // A sum or count at the end is not a stage: it says what to do with the items.
  const last = parts.at(-1)?.join(" ");
  const finish = last === "sum" || last === "count" ? parts.pop()![0] : "list";
  let items = source(parts[0]);
  for (const words of parts.slice(1)) items = stage(items, words);
  // Nothing has run yet. Only the lines below pull items through.
  if (finish === "sum") return String(items.reduce((a, b) => a + b, 0));
  if (finish === "count") return String(items.reduce((count) => count + 1, 0));
  return items.toArray().join(" ") || "(empty)";
}

for (const line of readFileSync(0, "utf8").split("\n")) {
  if (line.trim() !== "") console.log(run(line));
}
