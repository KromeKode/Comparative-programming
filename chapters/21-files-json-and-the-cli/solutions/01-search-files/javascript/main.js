// Prints the lines that contain a pattern, from each file named on the command line, or
// from stdin. ../README.md says what it should print.
import { readFileSync, writeSync } from "node:fs";

const USAGE = "usage: search [-icnv] PATTERN [FILE...]";
const STDIN_NAME = "(standard input)";
// Node's errors have the C names of the system's error codes, such as ENOENT, and
// messages of their own; these are the messages strerror gives for them.
const MESSAGES = {
  ENOENT: "No such file or directory",
  EISDIR: "Is a directory",
};

function usageError(message) {
  console.error(`search: ${message}\n${USAGE}`);
  process.exit(2);
}

// A copy of the bytes with A to Z made a to z, and every other byte as it was.
function lower(bytes) {
  return Buffer.from(
    bytes.map((byte) => (byte >= 0x41 && byte <= 0x5a ? byte + 0x20 : byte)),
  );
}

// Writes the lines of data that are selected, and returns how many there are.
function search(data, name, pattern, options) {
  const lines = [];
  for (let start = 0; start < data.length; ) {
    let end = data.indexOf(0x0a, start); // 0x0a is "\n"
    if (end === -1) end = data.length; // a last line without a newline
    lines.push(data.subarray(start, end));
    start = end + 1;
  }
  if (options.has("i")) pattern = lower(pattern);
  const prefix = name === null ? "" : `${name}:`;
  const out = [];
  let count = 0;
  lines.forEach((line, i) => {
    const found = (options.has("i") ? lower(line) : line).includes(pattern);
    if (found === options.has("v")) return;
    count++;
    if (!options.has("c")) {
      const number = options.has("n") ? `${i + 1}:` : "";
      out.push(Buffer.from(prefix + number), line, Buffer.from("\n"));
    }
  });
  if (options.has("c")) out.push(Buffer.from(`${prefix}${count}\n`));
  // writeSync, to file descriptor 1, writes the bytes before it returns.
  writeSync(1, Buffer.concat(out));
  return count;
}

const args = process.argv.slice(2);
const options = new Set();
// Options come first; the first argument that isn't one is the pattern.
while (args.length > 0 && args[0].startsWith("-") && args[0] !== "-") {
  const arg = args.shift();
  if (arg === "--") break;
  for (const letter of arg.slice(1)) {
    if (!"icnv".includes(letter)) usageError(`unknown option -${letter}`);
    options.add(letter);
  }
}
if (args.length === 0) usageError("no pattern");
const pattern = Buffer.from(args[0]);
const files = args.length > 1 ? args.slice(1) : ["-"];

let selected = 0;
let status = 0;
for (const file of files) {
  let data;
  try {
    data = readFileSync(file === "-" ? 0 : file); // a Buffer of bytes; 0 is stdin
  } catch (error) {
    console.error(`search: ${file}: ${MESSAGES[error.code] ?? error.message}`);
    status = 2;
    continue;
  }
  const name = files.length > 1 ? (file === "-" ? STDIN_NAME : file) : null;
  selected += search(data, name, pattern, options);
}
process.exitCode = status === 0 && selected === 0 ? 1 : status;
