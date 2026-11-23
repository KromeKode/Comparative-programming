// Reads a JSON file of books and prints a report on them, as JSON: how many there are,
// the oldest, the books with each tag, and the books with none. JSON.parse gives plain
// objects and arrays, and the checks that each book has the fields it needs are ours.
import { readFileSync } from "node:fs";

// Returns the book, or throws if value isn't one.
function bookFromJson(value) {
  if (typeof value !== "object" || value === null) {
    throw new Error("each book must be an object");
  }
  // The default applies when tags is missing, but not when it is null.
  const { title, author, year, tags = [] } = value;
  if (
    typeof title !== "string" ||
    typeof author !== "string" ||
    !Number.isInteger(year)
  ) {
    throw new Error("each book needs a title, an author, and a year");
  }
  if (!Array.isArray(tags) || !tags.every((tag) => typeof tag === "string")) {
    throw new Error(`${title}: tags must be a list of strings`);
  }
  return { title, author, year, tags };
}

function readBooks(text) {
  const data = JSON.parse(text); // throws a SyntaxError if it isn't JSON
  if (!Array.isArray(data?.books)) {
    throw new Error("expected an object with a list of books");
  }
  return data.books.map(bookFromJson);
}

function report(books) {
  let oldest = null; // the first of the oldest, when two have the same year
  for (const book of books) {
    if (oldest === null || book.year < oldest.year) oldest = book;
  }
  const tags = new Map();
  for (const book of books) {
    for (const tag of book.tags) {
      if (!tags.has(tag)) tags.set(tag, []);
      tags.get(tag).push(book.title);
    }
  }
  const sortedTags = [...tags].sort(([a], [b]) => (a < b ? -1 : a > b ? 1 : 0));
  return {
    count: books.length,
    oldest: oldest === null ? null : `${oldest.title}, by ${oldest.author}`,
    // An object keeps its keys in the order they went in, unless they look like array
    // indexes, such as "7": those come first, in order of their value.
    tags: Object.fromEntries(sortedTags),
    untagged: books
      .filter((book) => book.tags.length === 0)
      .map((book) => book.title),
  };
}

if (process.argv.length !== 3) {
  console.error("usage: books FILE");
  process.exit(2);
}
const path = process.argv[2];
let text;
try {
  text = readFileSync(path, "utf8");
} catch (error) {
  // Such as "ENOENT: no such file or directory, open 'books.json'", with the path.
  console.error(`books: ${error.message}`);
  process.exit(1);
}
let books;
try {
  books = readBooks(text);
} catch (error) {
  console.error(`books: ${path}: ${error.message}`);
  process.exit(1);
}
console.log(JSON.stringify(report(books), null, 2));
