// Reads arithmetic, one expression on each line, such as 1 + 2 * 3, and prints each with
// its parentheses written out, and its value: (1 + (2 * 3)) = 7. The numbers are whole
// and fit in 64 bits; the operators are + - * / % ^ and unary -, with ( and ). / and %
// round toward zero, as in C. An error is printed with its line and a caret under its
// column, the next line is read, and at the end, the exit status is 1.
import { readFileSync } from "node:fs";

class CalcError extends Error {
  readonly column: number; // counted in characters, from 1
  constructor(column: number, message: string) {
    super(message);
    this.column = column;
  }
}

// The lexer turns the text into tokens: numbers, and operators and parentheses, each
// with its column. A number is 64 bits, which is more than a number can hold exactly,
// so it is a bigint.
type Token =
  | { kind: "number"; text: string; value: bigint; column: number }
  | { kind: "operator"; text: string; column: number } // or a parenthesis
  | { kind: "end"; text: ""; column: number }; // after the last token

const MAX = 2n ** 63n - 1n;
const MIN = -(2n ** 63n);

function describe(token: Token): string {
  return token.kind === "end" ? "the end of the line" : `"${token.text}"`;
}

function tokens(line: string): Token[] {
  // Spread into an array of code points, so that i + 1 is the column.
  const chars = [...line];
  const found: Token[] = [];
  // Only ASCII digits go to BigInt, which would also take "0x1F", "+5", and spaces
  // around the digits, and says BigInt("") is 0n.
  const isDigit = (c: string | undefined) =>
    c !== undefined && c >= "0" && c <= "9";
  let i = 0;
  while (i < chars.length) {
    const c = chars[i];
    if (c === " " || c === "\t") {
      i += 1;
    } else if (isDigit(c)) {
      const start = i;
      while (isDigit(chars[i])) i += 1;
      const text = chars.slice(start, i).join("");
      const value = BigInt(text);
      if (value > MAX) {
        throw new CalcError(start + 1, "number too large for 64 bits");
      }
      found.push({ kind: "number", text, value, column: start + 1 });
    } else if ("+-*/%^()".includes(c)) {
      found.push({ kind: "operator", text: c, column: i + 1 });
      i += 1;
    } else {
      throw new CalcError(i + 1, `unexpected "${c}"`);
    }
  }
  found.push({ kind: "end", text: "", column: chars.length + 1 });
  return found;
}

// The parser turns the tokens into a tree, whose nodes are of three kinds. column is
// where the operator is, for an error such as overflow.
type Expr =
  | { kind: "number"; value: bigint }
  | { kind: "negate"; operand: Expr; column: number }
  | { kind: "binary"; op: string; left: Expr; right: Expr; column: number };

// How tightly each operator holds the expressions on its left and right: the higher,
// the tighter. Left is less than right for left-associative operators, so 1 - 2 - 3 is
// (1 - 2) - 3, and more for ^, so 2 ^ 3 ^ 2 is 2 ^ (3 ^ 2).
const INFIX = new Map<string, [number, number]>([
  ["+", [1, 2]],
  ["-", [1, 2]],
  ["*", [3, 4]],
  ["/", [3, 4]],
  ["%", [3, 4]],
  ["^", [8, 7]],
]);
// Unary minus holds less tightly than ^, so -2 ^ 2 is -(2 ^ 2), and more than *.
const PREFIX = 5;

class Parser {
  private readonly tokens: Token[];
  private at = 0;

  constructor(line: string) {
    this.tokens = tokens(line);
  }

  private next(): Token {
    const token = this.tokens[this.at];
    if (token.kind !== "end") this.at += 1;
    return token;
  }

  // Parses the longest expression whose operators hold at least minPower.
  private expression(minPower: number): Expr {
    const token = this.next();
    let left: Expr;
    if (token.kind === "number") {
      left = { kind: "number", value: token.value };
    } else if (token.text === "-") {
      const operand = this.expression(PREFIX);
      left = { kind: "negate", operand, column: token.column };
    } else if (token.text === "(") {
      left = this.expression(0);
      const close = this.next();
      if (close.text !== ")") {
        throw new CalcError(
          close.column,
          `expected ")", found ${describe(close)}`,
        );
      }
    } else {
      const found = describe(token);
      throw new CalcError(
        token.column,
        `expected a number, "(", or "-", found ${found}`,
      );
    }
    for (;;) {
      const token = this.tokens[this.at];
      const powers =
        token.kind === "operator" ? INFIX.get(token.text) : undefined;
      if (powers === undefined || powers[0] < minPower) return left;
      this.next();
      const right = this.expression(powers[1]);
      left = {
        kind: "binary",
        op: token.text,
        left,
        right,
        column: token.column,
      };
    }
  }

  parse(): Expr {
    const expr = this.expression(0);
    const token = this.next();
    if (token.kind !== "end") {
      throw new CalcError(
        token.column,
        `expected an operator, found ${describe(token)}`,
      );
    }
    return expr;
  }
}

function show(expr: Expr): string {
  switch (expr.kind) {
    case "number":
      return expr.value.toString();
    case "negate":
      return `(-${show(expr.operand)})`;
    case "binary":
      return `(${show(expr.left)} ${expr.op} ${show(expr.right)})`;
  }
}

// A bigint has no limit, so each result is checked against 64 bits by hand.
function fit(value: bigint, column: number): bigint {
  if (value < MIN || value > MAX) throw new CalcError(column, "overflow");
  return value;
}

// base ** exponent would give up only once the bigint grew too large, with a
// RangeError. This squares the base for each bit of the exponent, and stops at the
// first overflow.
function power(base: bigint, exponent: bigint, column: number): bigint {
  let result = 1n;
  while (exponent > 0n) {
    if (exponent % 2n === 1n) result = fit(result * base, column);
    exponent /= 2n;
    if (exponent > 0n) base = fit(base * base, column);
  }
  return result;
}

function evaluate(expr: Expr): bigint {
  switch (expr.kind) {
    case "number":
      return expr.value;
    case "negate":
      return fit(-evaluate(expr.operand), expr.column);
    case "binary": {
      const a = evaluate(expr.left);
      const b = evaluate(expr.right);
      const column = expr.column;
      if ((expr.op === "/" || expr.op === "%") && b === 0n) {
        throw new CalcError(column, "division by zero");
      }
      // A bigint's / and % round toward zero, as C's do.
      switch (expr.op) {
        case "+":
          return fit(a + b, column);
        case "-":
          return fit(a - b, column);
        case "*":
          return fit(a * b, column);
        case "/":
          return fit(a / b, column);
        case "%":
          return a % b;
        default: // "^"
          if (b < 0n) throw new CalcError(column, "negative exponent");
          return power(a, b, column);
      }
    }
  }
}

let status = 0;
const lines = readFileSync(0, "utf8").split("\n");
if (lines.at(-1) === "") lines.pop(); // the input ended with a newline, or was empty
lines.forEach((text, index) => {
  const line = text.endsWith("\r") ? text.slice(0, -1) : text;
  if (line.replaceAll(/[ \t]/g, "") === "") return;
  try {
    const expr = new Parser(line).parse();
    console.log(`${show(expr)} = ${evaluate(expr)}`);
  } catch (err) {
    if (!(err instanceof CalcError)) throw err;
    // The caret goes under the column. A tab before it stays a tab, so that the caret
    // lines up with the text whatever a tab's width is.
    const before = [...line].slice(0, err.column - 1);
    const pad = before.map((c) => (c === "\t" ? "\t" : " ")).join("");
    console.error(
      `calc: line ${index + 1}, column ${err.column}: ${err.message}`,
    );
    console.error(`  ${line}\n  ${pad}^`);
    status = 1;
  }
});
process.exitCode = status;
