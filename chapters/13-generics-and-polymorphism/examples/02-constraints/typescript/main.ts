// Money keeps whole cents, so adding never rounds.
class Money {
  readonly cents: number;

  constructor(cents: number) {
    this.cents = cents;
  }

  plus(other: Money): Money {
    return new Money(this.cents + other.cents);
  }

  toString(): string {
    const rest = String(this.cents % 100).padStart(2, "0");
    return `$${Math.trunc(this.cents / 100)}.${rest}`;
  }
}

// A constraint can't ask for +, because only numbers, bigints, and strings
// have it. So total takes an Adder: an object that knows how to add Ts.
interface Adder<T> {
  zero: T;
  add(a: T, b: T): T;
}

const numberAdder: Adder<number> = { zero: 0, add: (a, b) => a + b };
const moneyAdder: Adder<Money> = {
  zero: new Money(0),
  add: (a, b) => a.plus(b),
};

function total<T>(items: readonly T[], adder: Adder<T>): T {
  let sum = adder.zero;
  for (const item of items) sum = adder.add(sum, item);
  return sum;
}

// A compare function plays the same part for ordering, as it does for sort.
type Compare<T> = (a: T, b: T) => number;

const byValue: Compare<number | string> = (a, b) =>
  a < b ? -1 : a > b ? 1 : 0;
const byCents: Compare<Money> = (a, b) => a.cents - b.cents;

function describeRange<T>(items: readonly T[], compare: Compare<T>): string {
  let [low, high] = [items[0], items[0]];
  for (const item of items) {
    if (compare(item, low) < 0) low = item;
    if (compare(item, high) > 0) high = item;
  }
  return `${low} to ${high}`;
}

function list<T>(items: readonly T[]): string {
  return items.map(String).join(", ");
}

const numbers = [1, 2, 3];
const halves = [0.5, 0.25];
const prices = [new Money(150), new Money(225)];
console.log(`total of ${list(numbers)}: ${total(numbers, numberAdder)}`);
console.log(`total of ${list(halves)}: ${total(halves, numberAdder)}`);
console.log(`total of ${list(prices)}: ${total(prices, moneyAdder)}`);

const digits = [3, 9, 1];
const fruit = ["pear", "fig", "apple"];
const costs = [new Money(200), new Money(99)];
console.log(`range of ${list(digits)}: ${describeRange(digits, byValue)}`);
console.log(`range of ${list(fruit)}: ${describeRange(fruit, byValue)}`);
console.log(`range of ${list(costs)}: ${describeRange(costs, byCents)}`);
