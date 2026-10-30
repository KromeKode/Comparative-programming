const yesNo = (condition: boolean): string => (condition ? "yes" : "no");

const text = "  The quick  brown fox  ";

const trimmed = text.trim();
console.log(`trimmed: [${trimmed}]`);
// split(" ") would give an empty string between the two spaces; a regex splits on runs.
console.log("words:", trimmed.split(/\s+/).join("|"));
console.log("upper:", trimmed.toUpperCase());
console.log('contains "quick":', yesNo(trimmed.includes("quick")));
console.log('index of "brown":', trimmed.indexOf("brown")); // -1 if missing
console.log("replaced:", trimmed.replaceAll("quick", "slow"));
console.log('starts with "The":', yesNo(trimmed.startsWith("The")));

const fruits: string[] = ["banana", "apple", "Cherry"];
fruits.sort(); // compares UTF-16 code units
console.log("sorted:", fruits.join(" "));
fruits.sort((a, b) => {
  const x = a.toLowerCase();
  const y = b.toLowerCase();
  return x < y ? -1 : x > y ? 1 : 0;
});
console.log("sorted ignoring case:", fruits.join(" "));

export {};
