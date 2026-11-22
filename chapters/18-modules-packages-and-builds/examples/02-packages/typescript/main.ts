import { readFileSync } from "node:fs";

// A bare name is a package: Node looks for it in node_modules, where npm put
// it, and loads the file that its package.json "exports". Node removes the
// types from it as it would from a file of this package.
import { VERSION, longestWord, wordCount } from "textstats";

const text = readFileSync(0, "utf8");
console.log(`textstats ${VERSION}`);
console.log(`words: ${wordCount(text)}`);
console.log(`longest: ${longestWord(text)}`);
