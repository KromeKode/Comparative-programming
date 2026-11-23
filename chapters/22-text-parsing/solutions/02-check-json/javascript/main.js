// Reads one JSON text on stdin, strictly as RFC 8259 has it, and prints it compactly:
// no spaces, keys in the order they came, numbers as they were written, and strings
// written again with only the escapes they need. Or prints the first error's line and
// column, and exits with 1.
import { readFileSync } from "node:fs";

const MAX_DEPTH = 1000;

const SHORT = new Map([
  ['"', '\\"'],
  ["\\", "\\\\"],
  ["\b", "\\b"],
  ["\f", "\\f"],
  ["\n", "\\n"],
  ["\r", "\\r"],
  ["\t", "\\t"],
]);
const SIMPLE = new Map([
  ['"', '"'],
  ["\\", "\\"],
  ["/", "/"],
  ["b", "\b"],
  ["f", "\f"],
  ["n", "\n"],
  ["r", "\r"],
  ["t", "\t"],
]);

// s as a JSON string, with only the escapes it needs. JSON.stringify(s) writes the same,
// for a string with no lone surrogates; this shows what that is.
function quote(s) {
  let out = '"';
  for (const c of s) {
    const short = SHORT.get(c);
    if (short !== undefined) {
      out += short;
    } else if (c < " ") {
      out += "\\u" + c.charCodeAt(0).toString(16).padStart(4, "0");
    } else {
      out += c;
    }
  }
  return out + '"';
}

class JsonError extends Error {
  constructor(at, message) {
    super(message);
    this.at = at; // an index into the code points
  }
}

class Parser {
  // Code points, so that an index counts them, as columns do; a string's own indexes
  // count UTF-16 units.
  text;
  #i = 0;
  #depth = 0;

  constructor(text) {
    this.text = [...text];
  }

  // The code point at i, or "" at the end.
  #peek() {
    return this.text[this.#i] ?? "";
  }

  #fail(expected) {
    const c = this.#peek();
    const found = c === "" ? "the end of the input" : quote(c);
    return new JsonError(this.#i, `expected ${expected}, found ${found}`);
  }

  #space() {
    while ([" ", "\t", "\n", "\r"].includes(this.#peek())) this.#i += 1;
  }

  #startsWith(at, word) {
    return this.text.slice(at, at + word.length).join("") === word;
  }

  #value() {
    const c = this.#peek();
    if (c === "{" || c === "[") {
      this.#depth += 1;
      if (this.#depth > MAX_DEPTH) {
        throw new JsonError(this.#i, `nesting deeper than ${MAX_DEPTH}`);
      }
      const out = c === "{" ? this.#object() : this.#array();
      this.#depth -= 1;
      return out;
    }
    if (c === '"') return quote(this.#string());
    if (c === "-" || (c >= "0" && c <= "9")) return this.#number();
    for (const word of ["true", "false", "null"]) {
      if (this.#startsWith(this.#i, word)) {
        this.#i += word.length;
        return word;
      }
    }
    throw this.#fail("a value");
  }

  #object() {
    this.#i += 1; // the {
    this.#space();
    if (this.#peek() === "}") {
      this.#i += 1;
      return "{}";
    }
    const members = [];
    const keys = new Set();
    for (;;) {
      if (this.#peek() !== '"') {
        throw this.#fail(
          members.length === 0 ? 'a string key or "}"' : "a string key",
        );
      }
      const at = this.#i;
      const key = this.#string();
      if (keys.has(key)) throw new JsonError(at, `duplicate key ${quote(key)}`);
      keys.add(key);
      this.#space();
      if (this.#peek() !== ":") throw this.#fail('":"');
      this.#i += 1;
      this.#space();
      members.push(`${quote(key)}:${this.#value()}`);
      this.#space();
      if (this.#peek() === "}") {
        this.#i += 1;
        return `{${members.join(",")}}`;
      }
      if (this.#peek() !== ",") throw this.#fail('"," or "}"');
      this.#i += 1;
      this.#space();
    }
  }

  #array() {
    this.#i += 1; // the [
    this.#space();
    if (this.#peek() === "]") {
      this.#i += 1;
      return "[]";
    }
    const items = [];
    for (;;) {
      items.push(this.#value());
      this.#space();
      if (this.#peek() === "]") {
        this.#i += 1;
        return `[${items.join(",")}]`;
      }
      if (this.#peek() !== ",") throw this.#fail('"," or "]"');
      this.#i += 1;
      this.#space();
    }
  }

  #isDigit() {
    const c = this.#peek();
    return c >= "0" && c <= "9";
  }

  #digits() {
    if (!this.#isDigit()) throw this.#fail("a digit");
    while (this.#isDigit()) this.#i += 1;
  }

  #number() {
    const start = this.#i;
    if (this.#peek() === "-") this.#i += 1;
    if (this.#peek() === "0")
      this.#i += 1; // a 0 can't have digits after it
    else this.#digits();
    if (this.#peek() === ".") {
      this.#i += 1;
      this.#digits();
    }
    if (this.#peek() === "e" || this.#peek() === "E") {
      this.#i += 1;
      if (this.#peek() === "+" || this.#peek() === "-") this.#i += 1;
      this.#digits();
    }
    return this.text.slice(start, this.#i).join(""); // as it was written
  }

  // The four hex digits at text[at], or undefined if they aren't.
  #hex4(at) {
    const digits = this.text.slice(at, at + 4).join("");
    return /^[0-9a-fA-F]{4}$/.test(digits) ? parseInt(digits, 16) : undefined;
  }

  // The string at i, with its escapes undone.
  #string() {
    const start = this.#i;
    this.#i += 1; // the "
    let out = "";
    for (;;) {
      const c = this.#peek();
      if (c === "") throw new JsonError(start, "string is never closed");
      if (c === '"') {
        this.#i += 1;
        return out;
      }
      if (c < " ")
        throw new JsonError(this.#i, "control character in a string");
      if (c !== "\\") {
        out += c;
        this.#i += 1;
        continue;
      }
      const escape = this.text[this.#i + 1];
      if (escape === undefined)
        throw new JsonError(start, "string is never closed");
      const simple = SIMPLE.get(escape);
      if (simple !== undefined) {
        out += simple;
        this.#i += 2;
        continue;
      }
      let code = escape === "u" ? this.#hex4(this.#i + 2) : undefined;
      if (code === undefined) throw new JsonError(this.#i, "invalid escape");
      const at = this.#i;
      this.#i += 6;
      if (code >= 0xdc00 && code <= 0xdfff)
        throw new JsonError(at, "unpaired surrogate");
      if (code >= 0xd800 && code <= 0xdbff) {
        // A character above U+FFFF is two escapes, a high surrogate and a low one.
        const low = this.#startsWith(this.#i, "\\u")
          ? this.#hex4(this.#i + 2)
          : undefined;
        if (low === undefined || low < 0xdc00 || low > 0xdfff) {
          throw new JsonError(at, "unpaired surrogate");
        }
        code = 0x10000 + ((code - 0xd800) << 10) + (low - 0xdc00);
        this.#i += 6;
      }
      out += String.fromCodePoint(code);
    }
  }

  parse() {
    this.#space();
    const out = this.#value();
    this.#space();
    if (this.#i < this.text.length) throw this.#fail("the end of the input");
    return out;
  }
}

const parser = new Parser(readFileSync(0, "utf8"));
try {
  console.log(parser.parse());
} catch (err) {
  if (!(err instanceof JsonError)) throw err;
  // Lines end at "\n", and columns count characters from 1.
  const before = parser.text.slice(0, err.at);
  const line = before.filter((c) => c === "\n").length + 1;
  const column = err.at - before.lastIndexOf("\n");
  console.error(`jsoncheck: line ${line}, column ${column}: ${err.message}`);
  process.exitCode = 1;
}
