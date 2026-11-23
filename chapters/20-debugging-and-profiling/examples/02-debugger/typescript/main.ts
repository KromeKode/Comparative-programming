// Evaluates arithmetic with +, -, *, and parentheses: each argument, or each line of
// input if there are none. Section 20.5 runs it in a debugger, to watch the parser
// call itself. It expects well-formed expressions, and doesn't check.
import { readFileSync } from "node:fs";

// A recursive descent parser, with one method for each rule of the grammar.
class Parser {
  private readonly text: string;
  private pos = 0;

  constructor(text: string) {
    this.text = text;
  }

  // Skips spaces, and gives the next character, or "" at the end.
  private peek(): string {
    while (this.text[this.pos] === " ") this.pos++;
    return this.text[this.pos] ?? "";
  }

  // expr = term { ("+" | "-") term }
  expr(): number {
    let value = this.term();
    while (this.peek() === "+" || this.peek() === "-") {
      const op = this.text[this.pos++];
      const right = this.term();
      value = op === "+" ? value + right : value - right;
    }
    return value;
  }

  // term = factor { "*" factor }
  private term(): number {
    let value = this.factor();
    while (this.peek() === "*") {
      this.pos++;
      value *= this.factor();
    }
    return value;
  }

  // factor = number | "(" expr ")"
  private factor(): number {
    if (this.peek() === "(") {
      this.pos++;
      const value = this.expr();
      this.peek();
      this.pos++; // the ")"
      return value;
    }
    const start = this.pos;
    while (/[0-9]/.test(this.text[this.pos] ?? "")) this.pos++;
    return Number(this.text.slice(start, this.pos));
  }
}

function evaluate(text: string): number {
  return new Parser(text).expr();
}

const args = process.argv.slice(2);
const texts =
  args.length > 0 ? args : readFileSync(0, "utf8").split("\n").slice(0, -1);
for (const text of texts) {
  console.log(evaluate(text));
}
