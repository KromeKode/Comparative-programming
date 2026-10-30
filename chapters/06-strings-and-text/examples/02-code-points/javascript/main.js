const s = "Zo\u00eb \u{1F44D}\u{1F3FD}";
const hex = (n, width) =>
  n.toString(16).toUpperCase().padStart(width, "0");

const bytes = [...new TextEncoder().encode(s)].map((b) => hex(b, 2));
console.log("UTF-8 bytes:", bytes.join(" "));

// for...of and [...s] walk code points; s[i] and s.length count UTF-16 code units.
const codePoints = [...s].map((c) => "U+" + hex(c.codePointAt(0), 4));
console.log("code points:", codePoints.join(" "));

console.log("first 3 code points:", [...s].slice(0, 3).join(""));
