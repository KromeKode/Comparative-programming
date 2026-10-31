import { readFileSync } from "node:fs";

function shiftLetter(c, shift) {
  const base = c <= "Z" ? 65 : 97; // the code of "A" or "a"
  return String.fromCharCode(((c.charCodeAt(0) - base + shift) % 26) + base);
}

for (const raw of readFileSync(0, "utf8").split("\n")) {
  const line = raw.trim();
  if (line === "") continue;
  const space = line.indexOf(" ");
  const number = space === -1 ? line : line.slice(0, space);
  const text = space === -1 ? "" : line.slice(space + 1);
  const shift = ((Number(number) % 26) + 26) % 26; // JavaScript's % keeps the sign
  // replace with a function calls it for each match: here, each ASCII letter.
  console.log(text.replace(/[A-Za-z]/g, (c) => shiftLetter(c, shift)));
}
