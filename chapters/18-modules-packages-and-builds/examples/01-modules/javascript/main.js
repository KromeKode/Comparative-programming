import { readFileSync } from "node:fs";

// A relative import names the file, extension and all. Node runs textstats.js
// first, once, however many modules import it.
import { VERSION, longestWord, wordCount } from "./textstats.js";

const text = readFileSync(0, "utf8");
console.log(`textstats ${VERSION}`);
console.log(`words: ${wordCount(text)}`);
console.log(`longest: ${longestWord(text)}`);
