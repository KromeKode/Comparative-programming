// Prints each line of stdin that has a match for the pattern, as grep does, with a
// matcher of its own: the pattern becomes a tree, the tree becomes a program for a
// Thompson NFA, and the NFA reads each line once, keeping the set of states it could be
// in. So a line takes time in proportion to its length times the pattern's, whatever
// the pattern is.
import { readFileSync } from "node:fs";

// What a "\" can make a literal: the characters that mean something, inside brackets or
// out.
const SPECIAL = "\\.[]()*+?|^$-";

class PatternError extends Error {
  readonly at: number; // an index into the pattern's characters

  constructor(at: number, message: string) {
    super(message);
    this.at = at;
  }
}

// Code points, as numbers. Comparing strings compares UTF-16 units, in which "\u{ff5e}"
// comes after "🦊", whose first unit is 0xd83e.
const codePoints = (s: string): number[] =>
  Array.from(s, (c) => c.codePointAt(0)!);

// The tree. A character, a ".", and brackets are each a set of characters: "a" is the
// range a to a, and "." is the empty set, negated.
type Node =
  | { kind: "chars"; ranges: [number, number][]; negated: boolean }
  | { kind: "anchor"; anchor: "^" | "$" }
  | { kind: "concat"; items: Node[] }
  | { kind: "alternation"; options: Node[] }
  | { kind: "repeat"; item: Node; repeat: "*" | "+" | "?" };

// Reads alternation = concat ("|" concat)*, concat = repeat*, and repeat = atom [*+?].
class Parser {
  private readonly pattern: string[];
  private i = 0;

  constructor(pattern: string) {
    this.pattern = [...pattern]; // code points, so that an index counts characters
  }

  private peek(): string | undefined {
    return this.pattern[this.i]; // undefined at the end
  }

  parse(): Node {
    const node = this.alternation();
    if (this.i < this.pattern.length) {
      // alternation() stops early only at a ")".
      throw new PatternError(this.i, 'unmatched ")"');
    }
    return node;
  }

  private alternation(): Node {
    const options = [this.concat()];
    while (this.peek() === "|") {
      this.i++;
      options.push(this.concat());
    }
    return options.length === 1 ? options[0] : { kind: "alternation", options };
  }

  private concat(): Node {
    const items: Node[] = [];
    for (
      let c = this.peek();
      c !== undefined && c !== "|" && c !== ")";
      c = this.peek()
    ) {
      items.push(this.repeat());
    }
    return { kind: "concat", items };
  }

  private repeat(): Node {
    const c = this.peek();
    // At the start, or after "(", "|", "^", "$", or another "*", "+", or "?".
    if (c === "*" || c === "+" || c === "?") {
      throw new PatternError(this.i, "nothing to repeat");
    }
    if (c === "^" || c === "$") {
      this.i++;
      return { kind: "anchor", anchor: c };
    }
    const item = this.atom();
    const next = this.peek();
    if (next === "*" || next === "+" || next === "?") {
      this.i++;
      return { kind: "repeat", item, repeat: next };
    }
    return item;
  }

  private atom(): Node {
    const c = this.peek();
    if (c === "(") {
      const start = this.i;
      this.i++;
      const node = this.alternation();
      if (this.peek() !== ")") throw new PatternError(start, 'unclosed "("');
      this.i++;
      return node;
    }
    if (c === "[") return this.brackets();
    if (c === ".") {
      this.i++;
      return { kind: "chars", ranges: [], negated: true };
    }
    const code = this.char();
    return { kind: "chars", ranges: [[code, code]], negated: false };
  }

  // A character, or a "\" and the character it makes a literal, as a code point.
  private char(): number {
    const c = this.pattern[this.i]!;
    if (c !== "\\") {
      this.i++;
      return c.codePointAt(0)!;
    }
    if (this.i + 1 === this.pattern.length) {
      throw new PatternError(this.i, "trailing backslash");
    }
    const escaped = this.pattern[this.i + 1]!;
    if (!SPECIAL.includes(escaped)) {
      throw new PatternError(this.i, "invalid escape");
    }
    this.i += 2;
    return escaped.codePointAt(0)!;
  }

  private brackets(): Node {
    const start = this.i;
    this.i++; // the [
    const negated = this.peek() === "^";
    if (negated) this.i++;
    if (this.peek() === "]") throw new PatternError(start, "empty brackets");
    const ranges: [number, number][] = [];
    while (this.peek() !== "]") {
      if (this.peek() === undefined) {
        throw new PatternError(start, 'unclosed "["');
      }
      const at = this.i;
      const low = this.char();
      let high = low;
      // A "-" makes a range, unless it's last: "[a-]" is "a" and "-".
      const after = this.pattern[this.i + 1];
      if (this.peek() === "-" && after !== undefined && after !== "]") {
        this.i++;
        high = this.char();
        if (high < low) throw new PatternError(at, "range out of order");
      }
      ranges.push([low, high]);
    }
    this.i++; // the ]
    return { kind: "chars", ranges, negated };
  }
}

// The program. "chars" reads a character, if it's in the set, and goes on to the next
// instruction; "split" goes on to both x and y; "jump" goes on to x; "^" and "$" go on
// if the position is the start or the end of the line; and "match" is a match.
type Chars = { op: "chars"; ranges: [number, number][]; negated: boolean };
type Split = { op: "split"; x: number; y: number };
type Jump = { op: "jump"; x: number };
type Instruction = Chars | Split | Jump | { op: "^" | "$" | "match" };

function compile(node: Node, program: Instruction[]): void {
  const start = program.length;
  switch (node.kind) {
    case "chars":
      program.push({ op: "chars", ranges: node.ranges, negated: node.negated });
      break;
    case "anchor":
      program.push({ op: node.anchor });
      break;
    case "concat":
      for (const item of node.items) compile(item, program);
      break;
    case "alternation": {
      // split L1, L2; L1: first; jump END; L2: split L3, L4; L3: second; ...
      const jumps: Jump[] = [];
      for (const option of node.options.slice(0, -1)) {
        const split: Split = { op: "split", x: program.length + 1, y: 0 };
        program.push(split);
        compile(option, program);
        const jump: Jump = { op: "jump", x: 0 };
        jumps.push(jump);
        program.push(jump);
        split.y = program.length;
      }
      compile(node.options[node.options.length - 1]!, program);
      for (const jump of jumps) jump.x = program.length;
      break;
    }
    case "repeat":
      if (node.repeat === "*") {
        // L1: split L2, L3; L2: item; jump L1; L3:
        const split: Split = { op: "split", x: start + 1, y: 0 };
        program.push(split);
        compile(node.item, program);
        program.push({ op: "jump", x: start });
        split.y = program.length;
      } else if (node.repeat === "+") {
        // L1: item; split L1, L2; L2:
        compile(node.item, program);
        program.push({ op: "split", x: start, y: program.length + 1 });
      } else {
        // split L1, L2; L1: item; L2:
        const split: Split = { op: "split", x: start + 1, y: 0 };
        program.push(split);
        compile(node.item, program);
        split.y = program.length;
      }
  }
}

function compilePattern(pattern: string): Instruction[] {
  const program: Instruction[] = [];
  compile(new Parser(pattern).parse(), program);
  program.push({ op: "match" });
  return program;
}

// The "chars" instructions that pcs lead to without reading a character, at the
// position at, and whether one of them leads to "match".
function follow(
  program: Instruction[],
  pcs: number[],
  at: number,
  length: number,
): [number[], boolean] {
  const waiting: number[] = [];
  const seen = new Set<number>();
  const stack = [...pcs];
  let pc: number | undefined;
  while ((pc = stack.pop()) !== undefined) {
    if (seen.has(pc)) continue; // so that a loop such as (a*)* doesn't go around forever
    seen.add(pc);
    const instruction = program[pc]!;
    switch (instruction.op) {
      case "chars":
        waiting.push(pc);
        break;
      case "split":
        stack.push(instruction.y, instruction.x);
        break;
      case "jump":
        stack.push(instruction.x);
        break;
      case "^":
        if (at === 0) stack.push(pc + 1);
        break;
      case "$":
        if (at === length) stack.push(pc + 1);
        break;
      case "match":
        return [[], true];
    }
  }
  return [waiting, false];
}

function accepts(instruction: Instruction, c: number): boolean {
  if (instruction.op !== "chars") return false;
  const inside = instruction.ranges.some(
    ([low, high]) => low <= c && c <= high,
  );
  return inside !== instruction.negated;
}

// Whether a match starts anywhere in the line, of code points: instruction 0 starts at
// each position.
function search(program: Instruction[], line: number[]): boolean {
  let waiting: number[] = [];
  for (let at = 0; at <= line.length; at++) {
    const [reached, matched] = follow(
      program,
      [...waiting, 0],
      at,
      line.length,
    );
    if (matched) return true;
    const c = line[at]; // undefined at the end
    waiting = reached
      .filter((pc) => c !== undefined && accepts(program[pc]!, c))
      .map((pc) => pc + 1);
  }
  return false;
}

const args = process.argv.slice(2);
if (args.length !== 1) {
  console.error("usage: rematch PATTERN");
  process.exit(2);
}
let program: Instruction[] = [];
try {
  program = compilePattern(args[0]!);
} catch (err) {
  if (!(err instanceof PatternError)) throw err;
  console.error(`rematch: column ${err.at + 1}: ${err.message}`);
  process.exit(2);
}
const lines = readFileSync(0, "utf8").split("\n");
if (lines.at(-1) === "") lines.pop(); // what comes after the last "\n", or the empty input
const found = lines.filter((line) => search(program, codePoints(line)));
process.stdout.write(found.map((line) => line + "\n").join(""));
process.exitCode = found.length > 0 ? 0 : 1;
