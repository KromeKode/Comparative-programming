// A Set keeps insertion order, so sort to print in number order.
const show = (items) => [...items].sort((x, y) => x - y).join(" ");

const a = new Set([1, 2, 3, 4]);
const b = new Set([3, 4, 5]);
console.log("a:", show(a));
console.log("b:", show(b));
console.log("union:", show(a.union(b))); // these three methods arrived in ES2025
console.log("intersection:", show(a.intersection(b)));
console.log("a minus b:", show(a.difference(b)));
console.log("3 in a:", a.has(3) ? "yes" : "no");

console.log("unique of 3 1 3 2 1, sorted:", show(new Set([3, 1, 3, 2, 1])));

// Two arrays are equal only if they are the same object, so [0, 0] and [0, 0] would be
// two different members. Use a string such as "0,0" as the key instead.
const path = [
  [0, 0],
  [1, 0],
  [0, 0],
  [1, 1],
];
const visited = new Set(path.map(([x, y]) => `${x},${y}`));
console.log(`visited ${path.length} points, ${visited.size} distinct`);
console.log("(1, 0) visited:", visited.has("1,0") ? "yes" : "no");
