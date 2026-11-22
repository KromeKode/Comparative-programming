import { readFileSync } from "node:fs";

// Node runs .ts files by removing the types, and doesn't change import paths,
// so the import names the .ts file. tsc allows that with
// --allowImportingTsExtensions, when it only checks types and writes nothing.
import { VERSION, longestWord, wordCount } from "./textstats.ts";

const text = readFileSync(0, "utf8");
console.log(`textstats ${VERSION}`);
console.log(`words: ${wordCount(text)}`);
console.log(`longest: ${longestWord(text)}`);
