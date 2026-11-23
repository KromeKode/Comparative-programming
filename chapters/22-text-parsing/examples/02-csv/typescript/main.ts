// Reads CSV, as RFC 4180 describes it, and prints each record as a JSON list of strings,
// one record on each line. A field in quotes can hold commas, line breaks, and "" for a
// quote. Lines can end with "\r\n" or "\n". Every record must have as many fields as the
// first. At the first error, it prints the line and column, and the exit status is 1.
import { readFileSync } from "node:fs";

// A union of strings, not an enum: Node can run TypeScript only by stripping out its
// types, and an enum isn't just a type. A misspelled state is a type error.
type State =
  | "recordStart" // nothing of this record read yet
  | "fieldStart" // just after a comma
  | "unquoted" // in a field that doesn't start with a quote
  | "quoted" // in a field that does
  | "quote" // just after a quote in a quoted field: its end, or the first of ""
  | "cr"; // just after a "\r" outside quotes, which needs a "\n" next

class CsvError extends Error {
  constructor(line: number, column: number, message: string) {
    super(`line ${line}, column ${column}: ${message}`);
  }
}

const fields = (n: number): string => (n === 1 ? "1 field" : `${n} fields`);

// The records in text, one at a time. Throws a CsvError at the first error.
function* records(text: string): Generator<string[]> {
  let state: State = "recordStart";
  let record: string[] = [];
  let field = "";
  let width = 0; // how many fields the first record has, once it has ended
  let line = 1;
  let column = 0; // where char is: columns count characters, from 1
  let start: [number, number] = [0, 0]; // where a quoted field's quote is, or the "\r"
  let recordLine = 1;
  // for...of goes through code points, not UTF-16 units, so "🦊" is one character. The
  // "" after them stands for the end, so that the end goes through the machine as well.
  for (const char of [...text, ""]) {
    column += 1;
    if (state === "recordStart") recordLine = line;
    let ended = false; // whether char ends a record
    switch (state) {
      case "quoted":
        if (char === "") {
          throw new CsvError(...start, "quoted field is never closed");
        }
        if (char === '"') state = "quote";
        else field += char;
        break;
      case "cr":
        if (char !== "\n") {
          throw new CsvError(
            ...start,
            "a carriage return without a line feed after it",
          );
        }
        ended = true;
        break;
      case "recordStart":
      case "fieldStart":
      case "unquoted":
      case "quote":
        if (char === '"') {
          if (state === "unquoted") {
            throw new CsvError(line, column, "a quote in an unquoted field");
          }
          if (state === "quote") field += '"';
          else start = [line, column];
          state = "quoted";
        } else if (char === ",") {
          record.push(field);
          field = "";
          state = "fieldStart";
        } else if (char === "\n" || char === "") {
          // At the very start, or just after a line end, the end makes no record.
          ended = !(char === "" && state === "recordStart");
        } else if (char === "\r") {
          start = [line, column];
          state = "cr";
        } else if (state === "quote") {
          throw new CsvError(
            line,
            column,
            "expected a comma or a line end after a closing quote",
          );
        } else {
          field += char;
          state = "unquoted";
        }
        break;
      default: {
        // Here state's type is never, unless a case above is missing.
        const missing: never = state;
        throw new Error(`no case for ${String(missing)}`);
      }
    }
    if (ended) {
      record.push(field);
      field = "";
      if (width === 0) {
        width = record.length;
      } else if (record.length !== width) {
        const message = `record has ${fields(record.length)}, but the first has ${width}`;
        throw new CsvError(recordLine, 1, message);
      }
      yield record;
      record = [];
      state = "recordStart";
    }
    if (char === "\n") {
      line += 1;
      column = 0;
    }
  }
}

try {
  for (const record of records(readFileSync(0, "utf8"))) {
    console.log(JSON.stringify(record));
  }
} catch (err) {
  if (!(err instanceof CsvError)) throw err;
  console.error(`csv2json: ${err.message}`);
  process.exitCode = 1;
}
