import { createInterface } from "node:readline";

class CalcError extends Error {}

// Each number class has plus, minus, times, and dividedBy, and a static parse.
// evaluate below works with any class that has them; nothing checks that it
// does until a method is missing at run time.
const whole = /^-?[0-9]+$/;

class Int {
  constructor(value) {
    this.value = value;
  }

  static parse(text) {
    return whole.test(text) ? new Int(Number(text)) : undefined;
  }

  plus(other) {
    return new Int(this.value + other.value);
  }

  minus(other) {
    return new Int(this.value - other.value);
  }

  times(other) {
    return new Int(this.value * other.value);
  }

  // / gives a fraction; Math.trunc rounds it toward zero.
  dividedBy(other) {
    if (other.value === 0) throw new CalcError("division by zero");
    return new Int(Math.trunc(this.value / other.value));
  }

  toString() {
    return String(this.value);
  }
}

function gcd(a, b) {
  return b === 0 ? Math.abs(a) : gcd(b, a % b);
}

// Always in lowest terms, with a positive bottom. Make one with Frac.of.
class Frac {
  constructor(top, bottom) {
    this.top = top;
    this.bottom = bottom;
  }

  static of(top, bottom) {
    const sign = bottom < 0 ? -1 : 1;
    const divisor = gcd(top, bottom);
    // + 0 turns -0 into 0, so 0 never prints as "-0".
    return new Frac((sign * top) / divisor + 0, (sign * bottom) / divisor);
  }

  static parse(text) {
    const match = /^(-?[0-9]+)(?:\/([0-9]+))?$/.exec(text);
    if (match === null) return undefined;
    const bottom = Number(match[2] ?? "1");
    // "1/0" is not a fraction: the bottom must be above zero.
    return bottom > 0 ? Frac.of(Number(match[1]), bottom) : undefined;
  }

  plus(o) {
    return Frac.of(
      this.top * o.bottom + o.top * this.bottom,
      this.bottom * o.bottom,
    );
  }

  minus(o) {
    return Frac.of(
      this.top * o.bottom - o.top * this.bottom,
      this.bottom * o.bottom,
    );
  }

  times(o) {
    return Frac.of(this.top * o.top, this.bottom * o.bottom);
  }

  dividedBy(o) {
    if (o.top === 0) throw new CalcError("division by zero");
    return Frac.of(this.top * o.bottom, this.bottom * o.top);
  }

  toString() {
    return this.bottom === 1 ? `${this.top}` : `${this.top}/${this.bottom}`;
  }
}

// Always 0 to 6. % keeps the sign of the left side, so -1 % 7 is -1; adding
// 7 and taking % again fixes that.
class Mod7 {
  constructor(n) {
    this.value = ((n % 7) + 7) % 7;
  }

  static parse(text) {
    return whole.test(text) ? new Mod7(Number(text)) : undefined;
  }

  plus(other) {
    return new Mod7(this.value + other.value);
  }

  minus(other) {
    return new Mod7(this.value - other.value);
  }

  times(other) {
    return new Mod7(this.value * other.value);
  }

  // x to the power 5 is the inverse of x when 7 is prime: x * x^5 = x^6 = 1.
  dividedBy(other) {
    if (other.value === 0) throw new CalcError("division by zero");
    return this.times(new Mod7(other.value ** 5));
  }

  toString() {
    return String(this.value);
  }
}

// Run the tokens on a stack of numbers made by kind.parse.
function evaluate(tokens, kind) {
  const stack = [];
  for (const token of tokens) {
    if (["+", "-", "*", "/"].includes(token)) {
      const right = stack.pop();
      const left = stack.pop();
      if (left === undefined || right === undefined)
        throw new CalcError("not enough numbers");
      if (token === "+") stack.push(left.plus(right));
      if (token === "-") stack.push(left.minus(right));
      if (token === "*") stack.push(left.times(right));
      if (token === "/") stack.push(left.dividedBy(right));
    } else {
      const number = kind.parse(token);
      if (number === undefined) throw new CalcError(`bad token "${token}"`);
      stack.push(number);
    }
  }
  if (stack.length !== 1)
    throw new CalcError(`expected one result, found ${stack.length}`);
  return stack[0];
}

// A class is a value, so the class itself is the kind: it has a static parse.
const kinds = new Map([
  ["int", Int],
  ["frac", Frac],
  ["mod7", Mod7],
]);

for await (const line of createInterface({ input: process.stdin })) {
  const [kind, ...tokens] = line.split(/\s+/).filter((word) => word !== "");
  if (kind === undefined) continue;
  try {
    const numberClass = kinds.get(kind);
    if (numberClass === undefined)
      throw new CalcError(`unknown kind "${kind}"`);
    console.log(String(evaluate(tokens, numberClass)));
  } catch (error) {
    if (!(error instanceof CalcError)) throw error;
    console.log(`error: ${error.message}`);
  }
}
