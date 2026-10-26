import { readFileSync } from "node:fs";

const TABLE = [
  [1000, "M"],
  [900, "CM"],
  [500, "D"],
  [400, "CD"],
  [100, "C"],
  [90, "XC"],
  [50, "L"],
  [40, "XL"],
  [10, "X"],
  [9, "IX"],
  [5, "V"],
  [4, "IV"],
  [1, "I"],
];

function toRoman(n) {
  let out = "";
  for (const [value, symbol] of TABLE) {
    while (n >= value) {
      out += symbol; // take the biggest value that still fits
      n -= value;
    }
  }
  return out;
}

function letterValue(letter) {
  switch (letter) {
    case "I":
      return 1;
    case "V":
      return 5;
    case "X":
      return 10;
    case "L":
      return 50;
    case "C":
      return 100;
    case "D":
      return 500;
    case "M":
      return 1000;
    default:
      throw new Error(`not a Roman numeral letter: ${letter}`);
  }
}

function fromRoman(text) {
  let total = 0;
  for (let i = 0; i < text.length; i++) {
    const value = letterValue(text[i]);
    // A smaller letter before a bigger one is subtracted: IV is 5 - 1.
    if (i + 1 < text.length && value < letterValue(text[i + 1])) {
      total -= value;
    } else {
      total += value;
    }
  }
  return total;
}

for (const line of readFileSync(0, "utf8").split("\n")) {
  const text = line.trim();
  if (text === "") {
    continue;
  }
  if (/^\d+$/.test(text)) {
    console.log(`${text} -> ${toRoman(Number(text))}`);
  } else {
    console.log(`${text} -> ${fromRoman(text)}`);
  }
}
