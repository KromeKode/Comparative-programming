import { readFileSync } from "node:fs";

const MAX = 2 ** 31 - 1;
const MIN = -(2 ** 31);

// The base class for every way parseInt32 can fail.
class ParseError extends Error {}

class Empty extends ParseError {
  constructor() {
    super("empty input");
  }
}

class NoDigits extends ParseError {
  constructor() {
    super("no digits after the sign");
  }
}

class InvalidCharacter extends ParseError {
  constructor(character, position) {
    super(`invalid character '${character}' at position ${position}`);
    this.character = character;
    this.position = position;
  }
}

class TooLarge extends ParseError {
  constructor() {
    super(`too large, the maximum is ${MAX}`);
  }
}

class TooSmall extends ParseError {
  constructor() {
    super(`too small, the minimum is ${MIN}`);
  }
}

function parseInt32(text) {
  if (text === "") {
    throw new Empty();
  }
  const negative = text[0] === "-";
  const start = text[0] === "+" || text[0] === "-" ? 1 : 0;
  if (start === text.length) {
    throw new NoDigits();
  }
  // Check every character first, so a bad one is reported even when the
  // digits before it are already too large.
  for (let i = start; i < text.length; i++) {
    if (text[i] < "0" || text[i] > "9") {
      throw new InvalidCharacter(text[i], i + 1);
    }
  }
  // Stop growing once the number is past every limit. A number is exact only
  // up to 2 ** 53, so it must not grow past that.
  let magnitude = 0;
  for (let i = start; i < text.length; i++) {
    magnitude = Math.min(magnitude * 10 + Number(text[i]), MAX + 2);
  }
  if (!negative && magnitude > MAX) {
    throw new TooLarge();
  }
  if (negative && -magnitude < MIN) {
    throw new TooSmall();
  }
  // For "-0" this returns -0, which prints as 0.
  return negative ? -magnitude : magnitude;
}

const input = readFileSync(0, "utf8");
const lines = input.split("\n");
if (lines.at(-1) === "") {
  lines.pop();
}
for (const line of lines) {
  try {
    console.log(`"${line}" -> ${parseInt32(line)}`);
  } catch (error) {
    if (!(error instanceof ParseError)) {
      throw error;
    }
    console.log(`"${line}" -> error: ${error.message}`);
  }
}
