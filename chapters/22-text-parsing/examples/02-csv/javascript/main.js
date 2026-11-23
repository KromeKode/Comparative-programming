// Reads CSV, as RFC 4180 describes it, and prints each record as a JSON list of strings,
// one record on each line. A field in quotes can hold commas, line breaks, and "" for a
// quote. Lines can end with "\r\n" or "\n". Every record must have as many fields as the
// first. At the first error, it prints the line and column, and the exit status is 1.
import { readFileSync } from "node:fs";

// JavaScript has no enums. A frozen object of strings is the usual stand-in, though a
// misspelled State.QOUTED is undefined, not an error.
const State = Object.freeze({
  RECORD_START: "RECORD_START", // nothing of this record read yet
  FIELD_START: "FIELD_START", // just after a comma
  UNQUOTED: "UNQUOTED", // in a field that doesn't start with a quote
  QUOTED: "QUOTED", // in a field that does
  QUOTE: "QUOTE", // just after a quote in a quoted field: its end, or the first of ""
  CR: "CR", // just after a "\r" outside quotes, which needs a "\n" next
});

class CsvError extends Error {
  constructor(line, column, message) {
    super(`line ${line}, column ${column}: ${message}`);
  }
}

const fields = (n) => (n === 1 ? "1 field" : `${n} fields`);

// The records in text, one at a time. Throws a CsvError at the first error.
function* records(text) {
  let state = State.RECORD_START;
  let record = [];
  let field = "";
  let width = 0; // how many fields the first record has, once it has ended
  let line = 1;
  let column = 0; // where char is: columns count characters, from 1
  let start = [0, 0]; // where the quote that began a quoted field is, or the "\r"
  let recordLine = 1;
  // for...of goes through code points, not UTF-16 units, so "🦊" is one character. The
  // "" after them stands for the end, so that the end goes through the machine as well.
  for (const char of [...text, ""]) {
    column += 1;
    if (state === State.RECORD_START) recordLine = line;
    let ended = false; // whether char ends a record
    if (state === State.QUOTED) {
      if (char === "") {
        throw new CsvError(...start, "quoted field is never closed");
      }
      if (char === '"') state = State.QUOTE;
      else field += char;
    } else if (state === State.QUOTE && char === '"') {
      field += '"';
      state = State.QUOTED;
    } else if (state === State.CR) {
      if (char !== "\n") {
        throw new CsvError(
          ...start,
          "a carriage return without a line feed after it",
        );
      }
      ended = true;
    } else if (state === State.RECORD_START && char === "") {
      // the end, after a line end or at the start: no record
    } else if (char === '"' && state !== State.QUOTE) {
      if (state === State.UNQUOTED) {
        throw new CsvError(line, column, "a quote in an unquoted field");
      }
      start = [line, column];
      state = State.QUOTED;
    } else if (char === ",") {
      record.push(field);
      field = "";
      state = State.FIELD_START;
    } else if (char === "\n" || char === "") {
      ended = true;
    } else if (char === "\r") {
      start = [line, column];
      state = State.CR;
    } else if (state === State.QUOTE) {
      throw new CsvError(
        line,
        column,
        "expected a comma or a line end after a closing quote",
      );
    } else {
      field += char;
      state = State.UNQUOTED;
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
      state = State.RECORD_START;
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
