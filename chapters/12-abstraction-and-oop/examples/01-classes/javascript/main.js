function gcd(a, b) {
  return b === 0 ? Math.abs(a) : gcd(b, a % b);
}

// A fraction kept in lowest terms, with the sign on the numerator.
class Fraction {
  // # makes a field private. Code outside the class can't read or write it,
  // and trying to is a SyntaxError.
  #numerator;
  #denominator;

  constructor(numerator, denominator) {
    if (denominator === 0) {
      throw new RangeError("denominator is zero");
    }
    const sign = Math.sign(denominator);
    const divisor = gcd(numerator, denominator);
    this.#numerator = (sign * numerator) / divisor;
    this.#denominator = (sign * denominator) / divisor;
  }

  plus(other) {
    return new Fraction(
      this.#numerator * other.#denominator +
        other.#numerator * this.#denominator,
      this.#denominator * other.#denominator,
    );
  }

  // JavaScript has no operator overloading, so == and < can't be taught
  // about fractions. These methods do the work instead.
  equals(other) {
    return (
      this.#numerator === other.#numerator &&
      this.#denominator === other.#denominator
    );
  }

  compare(other) {
    return (
      this.#numerator * other.#denominator -
      other.#numerator * this.#denominator
    );
  }

  // Template strings and String() call toString.
  toString() {
    return this.#denominator === 1
      ? `${this.#numerator}`
      : `${this.#numerator}/${this.#denominator}`;
  }
}

const yesNo = (value) => (value ? "yes" : "no");

const [half, third] = [new Fraction(1, 2), new Fraction(1, 3)];
console.log(`1/2 + 1/3 = ${half.plus(third)}`);
console.log(`2/-4 is stored as ${new Fraction(2, -4)}`);
console.log(`6/3 is stored as ${new Fraction(6, 3)}`);
try {
  new Fraction(1, 0);
} catch (error) {
  console.log(`1/0: error: ${error.message}`);
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
