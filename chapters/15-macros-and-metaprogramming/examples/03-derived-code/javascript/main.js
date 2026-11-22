// A plain object can list its own keys as the program runs, so one function
// can compare any two objects field by field.
function equal(a, b) {
  const keys = Object.keys(a);
  return (
    keys.length === Object.keys(b).length &&
    keys.every((key) => a[key] === b[key])
  );
}

// Compares field by field, in the order the keys were added.
function compare(a, b) {
  for (const key of Object.keys(a)) {
    if (a[key] < b[key]) return -1;
    if (a[key] > b[key]) return 1;
  }
  return 0;
}

const dune = {
  title: "Dune",
  author: "Frank Herbert",
  year: 1965,
  inPrint: true,
};
console.log("fields:", Object.keys(dune).join(" "));
console.log("dune:", dune);

// === asks whether two objects are the same object, not whether they match.
const copy = { ...dune };
console.log("copy === dune:", copy === dune);
console.log("equal(copy, dune):", equal(copy, dune));

// Spreading copies every field, and a field written after it replaces one.
const reissue = { ...dune, year: 1966 };
console.log("reissue:", reissue);
console.log("equal(reissue, dune):", equal(reissue, dune));
console.log("dune < reissue:", compare(dune, reissue) < 0);

// A Set compares objects the way === does, so it keeps all three.
const books = [dune, copy, reissue];
console.log("distinct objects:", new Set(books).size);
console.log(
  "distinct books:",
  new Set(books.map((b) => JSON.stringify(b))).size,
);
console.log("json:", JSON.stringify(dune));
