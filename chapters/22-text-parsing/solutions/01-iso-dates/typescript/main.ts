// Copies stdin to stdout, with each date in it written as YYYY-MM-DD.
import { readFileSync } from "node:fs";

const MONTHS = [
  "January", "February", "March", "April", "May", "June",
  "July", "August", "September", "October", "November", "December",
]; // prettier-ignore

// Each month's name, or its first three letters: "Jan(?:uary)?", and so on.
const MONTH = MONTHS.map((name) =>
  name.length > 3 ? `${name.slice(0, 3)}(?:${name.slice(3)})?` : name,
).join("|");
const [D, M, Y] = ["([0-9]{1,2})", "([0-9]{1,2})", "([0-9]{4})"];
// In JavaScript, \b looks at ASCII letters, digits, and _ alone, and \d is only [0-9].
const DATE = new RegExp(
  String.raw`\b(?:${D}/${M}/${Y}|(${MONTH}) ${D}, ${Y}|${D} (${MONTH}) ${Y})\b`,
  "gu",
);

function daysIn(month: number, year: number): number {
  if (month === 2) {
    const leap = year % 4 === 0 && (year % 100 !== 0 || year % 400 === 0);
    return leap ? 29 : 28;
  }
  return [4, 6, 9, 11].includes(month) ? 30 : 31;
}

const monthOf = (name: string) =>
  MONTHS.findIndex((m) => m.startsWith(name)) + 1;

// replace calls this with the text that matched, then each group, which is undefined
// for the groups of the two forms that didn't match, then where the match is.
function rewrite(text: string, ...g: (string | undefined)[]): string {
  const [day, month, year] =
    g[0] !== undefined
      ? [Number(g[0]), Number(g[1]), Number(g[2])]
      : g[3] !== undefined
        ? [Number(g[4]), monthOf(g[3]), Number(g[5])]
        : [Number(g[6]), monthOf(g[7] ?? ""), Number(g[8])];
  if (month < 1 || month > 12 || day < 1 || day > daysIn(month, year)) {
    return text; // not a real date, so left as it was
  }
  const pad = (n: number, width: number) => String(n).padStart(width, "0");
  return `${pad(year, 4)}-${pad(month, 2)}-${pad(day, 2)}`;
}

const text = readFileSync(0, "utf8");
process.stdout.write(text.replace(DATE, rewrite));
