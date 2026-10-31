// Pretend files, so the example does not touch the disk.
const files = new Map<string, string>([
  ["good.txt", "1\n2\n3"],
  ["bad.txt", "4\nfive\n6"],
]);

// One error class per kind of failure, so callers can tell them apart.
class NotFound extends Error {}
class NotANumber extends Error {}

function readFile(name: string): string {
  const text = files.get(name);
  if (text === undefined) {
    throw new NotFound(`${name}: no such file`);
  }
  return text;
}

function parseNumber(text: string): number {
  if (!/^-?[0-9]+$/.test(text)) {
    throw new NotANumber(`"${text}" is not a number`);
  }
  return Number(text);
}

function sumFile(name: string): number {
  const text = readFile(name); // a NotFound passes straight through
  let total = 0;
  for (const [index, line] of text.split("\n").entries()) {
    try {
      total += parseNumber(line);
    } catch (error) {
      // Throw a new error and keep the old one as its cause.
      throw new Error(`${name}: line ${index + 1}`, { cause: error });
    }
  }
  return total;
}

// A missing file counts as 0, but any other error stops the sum.
function sumAll(names: string[]): number {
  let total = 0;
  for (const name of names) {
    try {
      total += sumFile(name);
    } catch (error) {
      if (!(error instanceof NotFound)) {
        throw error;
      }
    }
  }
  return total;
}

// Join an error's message with the messages of its causes. Anything can be
// thrown, so a cause has the type unknown until we check it.
function describe(error: unknown): string {
  const parts: string[] = [];
  let cause = error;
  while (cause instanceof Error) {
    parts.push(cause.message);
    cause = cause.cause;
  }
  return parts.join(": ");
}

function show(label: string, run: () => number): void {
  try {
    console.log(`  ${label} -> ${run()}`);
  } catch (error) {
    console.log(`  ${label} -> error: ${describe(error)}`);
  }
}

console.log("one file at a time:");
for (const name of ["good.txt", "bad.txt", "missing.txt"]) {
  show(name, () => sumFile(name));
}
console.log("several files, where a missing file counts as 0:");
show("good.txt + missing.txt", () => sumAll(["good.txt", "missing.txt"]));
show("good.txt + bad.txt", () => sumAll(["good.txt", "bad.txt"]));
