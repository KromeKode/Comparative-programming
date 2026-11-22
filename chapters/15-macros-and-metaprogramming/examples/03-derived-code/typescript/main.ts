// The type exists only for the compiler. When the program runs, a Book is a
// plain object, and its keys come from the object, not from this type.
type Book = {
  readonly title: string;
  readonly author: string;
  readonly year: number;
  readonly inPrint: boolean;
};

// keyof T is the compiler's list of T's keys. Object.keys returns string[],
// since an object can have more keys than its type says.
function keysOf<T extends object>(value: T): (keyof T)[] {
  return Object.keys(value) as (keyof T)[];
}

function equal<T extends object>(a: T, b: T): boolean {
  const keys = keysOf(a);
  return (
    keys.length === keysOf(b).length && keys.every((key) => a[key] === b[key])
  );
}

function compare<T extends object>(a: T, b: T): number {
  for (const key of keysOf(a)) {
    if (a[key] < b[key]) return -1;
    if (a[key] > b[key]) return 1;
  }
  return 0;
}

const dune: Book = {
  title: "Dune",
  author: "Frank Herbert",
  year: 1965,
  inPrint: true,
};
console.log("fields:", keysOf(dune).join(" "));
console.log("dune:", dune);

const copy: Book = { ...dune };
console.log("copy === dune:", copy === dune);
console.log("equal(copy, dune):", equal(copy, dune));

const reissue: Book = { ...dune, year: 1966 };
console.log("reissue:", reissue);
console.log("equal(reissue, dune):", equal(reissue, dune));
console.log("dune < reissue:", compare(dune, reissue) < 0);

const books = [dune, copy, reissue];
console.log("distinct objects:", new Set(books).size);
console.log(
  "distinct books:",
  new Set(books.map((b) => JSON.stringify(b))).size,
);
console.log("json:", JSON.stringify(dune));
