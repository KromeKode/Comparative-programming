import { readFileSync } from "node:fs";

// The fields are written once, here, as a book with every field at its
// default. The compiler works out the type Book from this value, and as the
// program runs, the type of each default says how to parse its column.
const DEFAULT_BOOK = {
  title: "",
  author: "",
  year: 0,
  pages: 0,
  in_print: false,
};

type Book = typeof DEFAULT_BOOK;
type Column = keyof Book;
type Value = Book[Column];

class BadValue extends Error {}

// One parser per type of default. Each one is given the column's name too,
// for its error message.
const PARSERS: Record<string, (text: string, name: string) => Value> = {
  string: (text) => text,
  number(text, name) {
    if (!/^-?[0-9]+$/.test(text)) {
      throw new BadValue(`${name}: "${text}" is not a whole number`);
    }
    return Number(text);
  },
  boolean(text, name) {
    if (text !== "true" && text !== "false") {
      throw new BadValue(`${name}: "${text}" is not true or false`);
    }
    return text === "true";
  },
};

const isColumn = (name: string): name is Column =>
  Object.hasOwn(DEFAULT_BOOK, name);

const show = (value: Value): string =>
  typeof value === "string" ? `"${value}"` : String(value);

function describe(book: Book): string {
  const parts = Object.entries(book).map(
    ([name, value]) => `${name}=${show(value)}`,
  );
  return `Book(${parts.join(", ")})`;
}

function readBook(columns: Column[], values: string[]): Book {
  if (values.length !== columns.length) {
    throw new BadValue(
      `expected ${columns.length} values, got ${values.length}`,
    );
  }
  // The compiler can't tell which field a name from the input picks, so it
  // sees this object as a map from column names to values.
  const book: Record<Column, Value> = { ...DEFAULT_BOOK };
  for (const [i, name] of columns.entries()) {
    book[name] = PARSERS[typeof DEFAULT_BOOK[name]](values[i], name);
  }
  return book as Book;
}

const plural = (count: number, word: string): string =>
  `${count} ${word}${count === 1 ? "" : "s"}`;

function main(): void {
  const lines = readFileSync(0, "utf8")
    .split("\n")
    .filter((line) => line.trim() !== "");
  if (lines.length === 0) return;
  const split = (line: string): string[] =>
    line.split(",").map((part) => part.trim());
  const columns: Column[] = [];
  for (const name of split(lines[0])) {
    if (!isColumn(name)) {
      console.log(`unknown column: ${name}`);
      return;
    }
    if (columns.includes(name)) {
      console.log(`repeated column: ${name}`);
      return;
    }
    columns.push(name);
  }

  let books = 0;
  let bad = 0;
  for (const [i, line] of lines.slice(1).entries()) {
    try {
      console.log(describe(readBook(columns, split(line))));
      books++;
    } catch (error) {
      if (!(error instanceof BadValue)) throw error;
      console.log(`row ${i + 1}: ${error.message}`);
      bad++;
    }
  }
  console.log(`${plural(books, "book")}, ${plural(bad, "bad row")}`);
}

main();
