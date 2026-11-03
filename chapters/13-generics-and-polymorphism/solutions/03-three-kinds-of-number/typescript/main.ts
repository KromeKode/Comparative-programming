import { createInterface } from "node:readline";

class CalcError extends Error {}

// What evaluate needs from a number type. N extends Arithmetic<N> makes every
// method take and return the same type. dividedBy throws CalcError when
// dividing by zero.
interface Arithmetic<N> {
  plus(other: N): N;
  minus(other: N): N;
  times(other: N): N;
  dividedBy(other: N): N;
  toString(): string;
}

// Parsing makes a new N, so it can't be a method of an N. Each class below
// has a static parse, and the class itself is passed in as the NumberKind.
interface NumberKind<N> {
  parse(text: string): N | undefined;
}

const whole = /^-?[0-9]+$/;

class Int implements Arithmetic<Int> {
  readonly value: number;

  constructor(value: number) {
    this.value = value;
  }

  static parse(text: string): Int | undefined {
    return whole.test(text) ? new Int(Number(text)) : undefined;
  }

  plus(other: Int): Int {
    return new Int(this.value + other.value);
  }

  minus(other: Int): Int {
    return new Int(this.value - other.value);
  }

  times(other: Int): Int {
    return new Int(this.value * other.value);
  }

  // / gives a fraction; Math.trunc rounds it toward zero.
  dividedBy(other: Int): Int {
    if (other.value === 0) throw new CalcError("division by zero");
    return new Int(Math.trunc(this.value / other.value));
  }

  toString(): string {
    return String(this.value);
  }
}

function gcd(a: number, b: number): number {
  return b === 0 ? Math.abs(a) : gcd(b, a % b);
}

// Always in lowest terms, with a positive bottom. Make one with Frac.of.
class Frac implements Arithmetic<Frac> {
  readonly top: number;
  readonly bottom: number;

  private constructor(top: number, bottom: number) {
    this.top = top;
    this.bottom = bottom;
  }

  static of(top: number, bottom: number): Frac {
    const sign = bottom < 0 ? -1 : 1;
    const divisor = gcd(top, bottom);
    // + 0 turns -0 into 0, so 0 never prints as "-0".
    return new Frac((sign * top) / divisor + 0, (sign * bottom) / divisor);
  }

  static parse(text: string): Frac | undefined {
    const match = /^(-?[0-9]+)(?:\/([0-9]+))?$/.exec(text);
    if (match === null) return undefined;
    const bottom = Number(match[2] ?? "1");
    // "1/0" is not a fraction: the bottom must be above zero.
    return bottom > 0 ? Frac.of(Number(match[1]), bottom) : undefined;
  }

  plus(o: Frac): Frac {
    return Frac.of(
      this.top * o.bottom + o.top * this.bottom,
      this.bottom * o.bottom,
    );
  }

  minus(o: Frac): Frac {
    return Frac.of(
      this.top * o.bottom - o.top * this.bottom,
      this.bottom * o.bottom,
    );
  }

  times(o: Frac): Frac {
    return Frac.of(this.top * o.top, this.bottom * o.bottom);
  }

  dividedBy(o: Frac): Frac {
    if (o.top === 0) throw new CalcError("division by zero");
    return Frac.of(this.top * o.bottom, this.bottom * o.top);
  }

  toString(): string {
    return this.bottom === 1 ? `${this.top}` : `${this.top}/${this.bottom}`;
  }
}

// Always 0 to 6. % keeps the sign of the left side, so -1 % 7 is -1; adding
// 7 and taking % again fixes that.
class Mod7 implements Arithmetic<Mod7> {
  readonly value: number;

  constructor(n: number) {
    this.value = ((n % 7) + 7) % 7;
  }

  static parse(text: string): Mod7 | undefined {
    return whole.test(text) ? new Mod7(Number(text)) : undefined;
  }

  plus(other: Mod7): Mod7 {
    return new Mod7(this.value + other.value);
  }

  minus(other: Mod7): Mod7 {
    return new Mod7(this.value - other.value);
  }

  times(other: Mod7): Mod7 {
    return new Mod7(this.value * other.value);
  }

  // x to the power 5 is the inverse of x when 7 is prime: x * x^5 = x^6 = 1.
  dividedBy(other: Mod7): Mod7 {
    if (other.value === 0) throw new CalcError("division by zero");
    return this.times(new Mod7(other.value ** 5));
  }

  toString(): string {
    return String(this.value);
  }
}

// Run the tokens on a stack of N. The same code works for every number type.
function evaluate<N extends Arithmetic<N>>(
  tokens: string[],
  kind: NumberKind<N>,
): N {
  const stack: N[] = [];
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

// The classes themselves are the kinds: each has a static parse.
const kinds = new Map<string, (tokens: string[]) => string>([
  ["int", (tokens) => evaluate(tokens, Int).toString()],
  ["frac", (tokens) => evaluate(tokens, Frac).toString()],
  ["mod7", (tokens) => evaluate(tokens, Mod7).toString()],
]);

for await (const line of createInterface({ input: process.stdin })) {
  const [kind, ...tokens] = line.split(/\s+/).filter((word) => word !== "");
  if (kind === undefined) continue;
  try {
    const handle = kinds.get(kind);
    if (handle === undefined) throw new CalcError(`unknown kind "${kind}"`);
    console.log(handle(tokens));
  } catch (error) {
    if (!(error instanceof CalcError)) throw error;
    console.log(`error: ${error.message}`);
  }
}
