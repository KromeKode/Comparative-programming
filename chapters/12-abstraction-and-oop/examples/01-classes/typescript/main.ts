function gcd(a: number, b: number): number {
  return b === 0 ? Math.abs(a) : gcd(b, a % b);
}

// A fraction kept in lowest terms, with the sign on the numerator.
class Fraction {
  // private is checked by the compiler only. JavaScript's # fields are
  // private at run time too; TypeScript accepts both.
  private readonly numerator: number;
  private readonly denominator: number;

  constructor(numerator: number, denominator: number) {
    if (denominator === 0) {
      throw new RangeError("denominator is zero");
    }
    const sign = Math.sign(denominator);
    const divisor = gcd(numerator, denominator);
    this.numerator = (sign * numerator) / divisor;
    this.denominator = (sign * denominator) / divisor;
  }

  plus(other: Fraction): Fraction {
    return new Fraction(
      this.numerator * other.denominator + other.numerator * this.denominator,
      this.denominator * other.denominator,
    );
  }

  // No operator overloading, so equality and ordering are methods.
  equals(other: Fraction): boolean {
    return (
      this.numerator === other.numerator &&
      this.denominator === other.denominator
    );
  }

  compare(other: Fraction): number {
    return (
      this.numerator * other.denominator - other.numerator * this.denominator
    );
  }

  toString(): string {
    return this.denominator === 1
      ? `${this.numerator}`
      : `${this.numerator}/${this.denominator}`;
  }
}

const yesNo = (value: boolean): string => (value ? "yes" : "no");

const [half, third] = [new Fraction(1, 2), new Fraction(1, 3)];
console.log(`1/2 + 1/3 = ${half.plus(third)}`);
console.log(`2/-4 is stored as ${new Fraction(2, -4)}`);
console.log(`6/3 is stored as ${new Fraction(6, 3)}`);
try {
  new Fraction(1, 0);
} catch (error) {
  if (error instanceof RangeError) {
    console.log(`1/0: error: ${error.message}`);
  }
}
console.log(`1/2 == 2/4: ${yesNo(half.equals(new Fraction(2, 4)))}`);
console.log(`1/3 < 1/2: ${yesNo(third.compare(half) < 0)}`);
const fractions = [
  new Fraction(3, 4),
  third,
  new Fraction(-1, 2),
  new Fraction(2, 4),
];
fractions.sort((a, b) => a.compare(b));
console.log(`sorted: ${fractions.join(", ")}`);
