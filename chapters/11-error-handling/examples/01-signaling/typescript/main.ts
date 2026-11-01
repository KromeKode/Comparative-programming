const inputs = ["42", "4x2", "", "99999999999999999999"];

// Number never throws. It returns NaN, "not a number", when it cannot parse,
// and the empty string becomes 0.
console.log("Number:");
for (const text of inputs) {
  console.log(`  ${JSON.stringify(text)} -> ${Number(text)}`);
}

// parseInt stops at the first character that is not a digit.
console.log("parseInt:");
for (const text of inputs) {
  console.log(`  ${JSON.stringify(text)} -> ${parseInt(text, 10)}`);
}

// A strict parser has to be written by hand, and it throws. Nothing in its
// type says so: TypeScript has no way to declare what a function throws.
function parseInteger(text: string): number {
  if (!/^[+-]?\d+$/.test(text)) {
    throw new SyntaxError(`not an integer: ${JSON.stringify(text)}`);
  }
  const value = Number(text);
  if (!Number.isSafeInteger(value)) {
    throw new RangeError(`too large to be exact: ${text}`);
  }
  return value;
}

console.log("parseInteger:");
for (const text of inputs) {
  try {
    console.log(`  ${JSON.stringify(text)} -> ${parseInteger(text)}`);
  } catch (error) {
    // A catch clause gets `unknown`, so check what was thrown before using it.
    if (!(error instanceof Error)) throw error;
    console.log(`  ${JSON.stringify(text)} -> ${error.name}: ${error.message}`);
  }
}
