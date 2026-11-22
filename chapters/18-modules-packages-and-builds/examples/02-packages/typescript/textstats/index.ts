// The textstats module. Every file is a module with names of its own, and only
// the names marked export can be imported by other files.

export const VERSION = "1.0";

// Not exported, so private to this file.
function words(text: string): string[] {
  return text.match(/[A-Za-z]+/g) ?? [];
}

// The number of words in text. A word is a run of ASCII letters.
export function wordCount(text: string): number {
  return words(text).length;
}

// The first of the longest words in text.
export function longestWord(text: string): string {
  return words(text).reduce((best, word) => (word.length > best.length ? word : best), "");
}
