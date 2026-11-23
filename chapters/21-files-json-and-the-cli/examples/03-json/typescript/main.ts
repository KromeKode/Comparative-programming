// Reads a JSON file of books and prints a report on them, as JSON: how many there are,
// the oldest, the books with each tag, and the books with none. JSON.parse returns any,
// which the type checker trusts with no checks at all, so this treats it as unknown,
// and checks that each book has the fields it needs before calling it a Book.
import { readFileSync } from "node:fs";

interface Book {
  title: string;
  author: string;
  year: number;
  tags: string[];
}

interface Report {
  count: number;
  oldest: string | null;
  tags: Record<string, string[]>;
  untagged: string[];
}

// Returns the book, or throws if value isn't one.
function bookFromJson(value: unknown): Book {
  if (typeof value !== "object" || value === null) {
    throw new Error("each book must be an object");
  }
  const { title, author, year, tags = [] } = value as Record<string, unknown>;
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
  // Number.isInteger doesn't narrow unknown to number, so year needs a cast.
  return { title, author, year: year as number, tags };
}

function readBooks(text: string): Book[] {
  const data: unknown = JSON.parse(text);
  const books = (data as { books?: unknown } | null)?.books;
  if (!Array.isArray(books)) {
    throw new Error("expected an object with a list of books");
  }
  return books.map(bookFromJson);
}

function report(books: Book[]): Report {
  let oldest: Book | null = null; // the first of the oldest, when two have the same year
  for (const book of books) {
    if (oldest === null || book.year < oldest.year) oldest = book;
  }
  const tags = new Map<string, string[]>();
  for (const book of books) {
    for (const tag of book.tags) {
      const titles = tags.get(tag) ?? [];
      titles.push(book.title);
      tags.set(tag, titles);
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
let text: string;
try {
  text = readFileSync(path, "utf8");
} catch (error) {
  // Such as "ENOENT: no such file or directory, open 'books.json'", with the path.
  console.error(`books: ${(error as Error).message}`);
  process.exit(1);
}
let books: Book[];
try {
  books = readBooks(text);
} catch (error) {
  console.error(`books: ${path}: ${(error as Error).message}`);
  process.exit(1);
}
console.log(JSON.stringify(report(books), null, 2));
