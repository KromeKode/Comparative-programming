import { readFileSync } from "node:fs";

// The fields are written once, here, as a book with every field at its
// default. The type of each default says how to parse that column.
const DEFAULT_BOOK = {
  title: "",
  author: "",
  year: 0,
  pages: 0,
  in_print: false,
};

class BadValue extends Error {}

// One parser per type of default. Each one is given the column's name too,
// for its error message.
const PARSERS = {
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

const show = (value) =>
  typeof value === "string" ? `"${value}"` : String(value);

// Object.entries lists the fields in the order they were written.
function describe(book) {
  const parts = Object.entries(book).map(
    ([name, value]) => `${name}=${show(value)}`,
  );
  return `Book(${parts.join(", ")})`;
}

// A row becomes a copy of the default book, with each column's field set
// by its name.
function readBook(columns, values) {
  if (values.length !== columns.length) {
    throw new BadValue(
      `expected ${columns.length} values, got ${values.length}`,
    );
  }
  const book = { ...DEFAULT_BOOK };
  for (const [i, name] of columns.entries()) {
    book[name] = PARSERS[typeof DEFAULT_BOOK[name]](values[i], name);
  }
  return book;
}

const plural = (count, word) => `${count} ${word}${count === 1 ? "" : "s"}`;

function main() {
  const lines = readFileSync(0, "utf8")
    .split("\n")
    .filter((line) => line.trim() !== "");
  if (lines.length === 0) return;
  const split = (line) => line.split(",").map((part) => part.trim());
  const columns = split(lines[0]);
  for (const [i, name] of columns.entries()) {
    if (!Object.hasOwn(DEFAULT_BOOK, name)) {
      console.log(`unknown column: ${name}`);
      return;
    }
    if (columns.slice(0, i).includes(name)) {
      console.log(`repeated column: ${name}`);
      return;
    }
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
