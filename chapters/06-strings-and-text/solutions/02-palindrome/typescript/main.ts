import { readFileSync } from "node:fs";

const isAscii = (c: string): boolean => c.codePointAt(0)! <= 0x7f;

// [...text] splits by code point. text.split("") would split by UTF-16 code unit,
// and reversing that tears every emoji in half.
function isPalindrome(text: string): boolean {
  const kept = [...text]
    .filter((c) => !isAscii(c) || /[A-Za-z0-9]/.test(c))
    .map((c) => (isAscii(c) ? c.toLowerCase() : c));
  return kept.join("") === kept.toReversed().join("");
}

for (const line of readFileSync(0, "utf8").split("\n")) {
  const text = line.trim();
  if (text === "") continue;
  console.log(
    `${text}: ${isPalindrome(text) ? "palindrome" : "not a palindrome"}`,
  );
}
