// A JavaScript string is a sequence of UTF-16 code units.
const s = "Zo\u00eb \u{1F44D}\u{1F3FD}";

console.log("text:", s);
console.log(`built-in length: ${s.length} (UTF-16 code units)`);
console.log("UTF-8 bytes:", new TextEncoder().encode(s).length);
console.log("UTF-16 code units:", s.length);
console.log("code points:", [...s].length); // spreading a string splits it by code point

// Intl.Segmenter finds the characters a reader sees: grapheme clusters.
const segmenter = new Intl.Segmenter("en", { granularity: "grapheme" });
console.log("graphemes:", [...segmenter.segment(s)].length);
