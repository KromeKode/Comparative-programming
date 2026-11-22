import { readFileSync } from "node:fs";

class ExpandError extends Error {}

// Each macro has params, or null if it takes no parentheses, and a body.
const macros = new Map();

const isWordChar = (c) => /\w/.test(c);

const isName = (word) => /^[A-Za-z_]\w*$/.test(word);

function wordEnd(text, start) {
  let end = start;
  while (end < text.length && isWordChar(text[end])) end++;
  return end;
}

// The index just past the string that starts at text[start], a quote.
function stringEnd(text, start) {
  const close = text.indexOf('"', start + 1);
  return close < 0 ? text.length : close + 1;
}

// Splits the arguments of a call at its top-level commas, and trims them.
// Returns them with the index just past the ")".
function splitArgs(text, open, name) {
  const args = [];
  let depth = 0;
  let start = open + 1;
  let i = open + 1;
  while (i < text.length) {
    const c = text[i];
    if (c === '"') {
      i = stringEnd(text, i);
      continue;
    }
    if (c === "(") {
      depth++;
    } else if (c === ")" && depth > 0) {
      depth--;
    } else if (c === ")") {
      args.push(text.slice(start, i).trim());
      return [args, i + 1];
    } else if (c === "," && depth === 0) {
      args.push(text.slice(start, i).trim());
      start = i + 1;
    }
    i++;
  }
  throw new ExpandError(`unterminated call to ${name}`);
}

// The macro's body, with each parameter replaced by its argument.
function substitute(macro, args, disabled) {
  const raw = new Map();
  macro.params.forEach((param, p) => raw.set(param, args[p]));
  const expanded = new Map(); // only the arguments used
  const body = macro.body;
  let out = "";
  let i = 0;
  while (i < body.length) {
    let end = i + 1;
    if (body[i] === '"') {
      end = stringEnd(body, i);
    } else if (body[i] === "#") {
      // # and a parameter make a string of the argument as written.
      let start = i + 1;
      while (body[start] === " ") start++;
      const stop = wordEnd(body, start);
      const arg = raw.get(body.slice(start, stop));
      if (arg !== undefined) {
        out += `"${arg}"`;
        i = stop;
        continue;
      }
    } else if (isWordChar(body[i])) {
      end = wordEnd(body, i);
      const word = body.slice(i, end);
      const arg = raw.get(word);
      if (arg !== undefined) {
        let value = expanded.get(word);
        if (value === undefined) {
          value = expand(arg, disabled);
          expanded.set(word, value);
        }
        out += value;
        i = end;
        continue;
      }
    }
    out += body.slice(i, end);
    i = end;
  }
  return out;
}

// Expands every macro in text, except those in disabled.
function expand(text, disabled) {
  let out = "";
  let i = 0;
  while (i < text.length) {
    let end = i + 1;
    if (text[i] === '"') {
      end = stringEnd(text, i);
    } else if (isWordChar(text[i])) {
      end = wordEnd(text, i);
      const word = text.slice(i, end);
      const macro = macros.get(word);
      if (macro !== undefined && !disabled.has(word)) {
        // Inside its own expansion, a macro's name is left alone.
        const inner = new Set([...disabled, word]);
        if (macro.params === null) {
          out += expand(macro.body, inner);
          i = end;
          continue;
        }
        let paren = end;
        while (text[paren] === " ") paren++;
        if (text[paren] === "(") {
          const [found, after] = splitArgs(text, paren, word);
          const count = macro.params.length;
          // NOW() has one argument, "", which is no arguments at all.
          const empty = count === 0 && found.length === 1 && found[0] === "";
          const args = empty ? [] : found;
          if (args.length !== count) {
            const noun = count === 1 ? "argument" : "arguments";
            throw new ExpandError(
              `${word} takes ${count} ${noun}, got ${args.length}`,
            );
          }
          out += expand(substitute(macro, args, disabled), inner);
          i = after;
          continue;
        }
      }
    }
    out += text.slice(i, end);
    i = end;
  }
  return out;
}

function define(rest) {
  const end = wordEnd(rest, 0);
  const name = rest.slice(0, end);
  if (!isName(name)) throw new ExpandError("bad #define");
  if (rest[end] !== "(") {
    macros.set(name, { params: null, body: rest.slice(end).trim() });
    return;
  }
  const close = rest.indexOf(")", end);
  if (close < 0) throw new ExpandError("bad #define");
  const inside = rest.slice(end + 1, close);
  const params =
    inside.trim() === "" ? [] : inside.split(",").map((param) => param.trim());
  if (!params.every(isName) || new Set(params).size !== params.length) {
    throw new ExpandError("bad #define");
  }
  macros.set(name, { params, body: rest.slice(close + 1).trim() });
}

// Returns the line to print, or null for a directive.
function run(line) {
  const whole = line.trim();
  const space = whole.indexOf(" ");
  const directive = space < 0 ? whole : whole.slice(0, space);
  const rest = space < 0 ? "" : whole.slice(space + 1).trim();
  if (directive === "#define") {
    define(rest);
    return null;
  }
  if (directive === "#undef") {
    if (!isName(rest)) throw new ExpandError("bad #undef");
    macros.delete(rest);
    return null;
  }
  return expand(line, new Set());
}

function main() {
  const lines = readFileSync(0, "utf8").split("\n");
  if (lines.at(-1) === "") lines.pop(); // after the newline that ends the input
  for (const line of lines) {
    try {
      const result = run(line);
      if (result !== null) console.log(result);
    } catch (error) {
      if (!(error instanceof ExpandError)) throw error;
      console.log(`error: ${error.message}`);
    }
  }
}

main();
