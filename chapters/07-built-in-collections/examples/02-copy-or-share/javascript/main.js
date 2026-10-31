const show = (value) =>
  Array.isArray(value) ? `[${value.map(show).join(", ")}]` : `${value}`;

// The function gets a reference to the same array.
function setFirst(items) {
  items[0] = -1;
}

const a = [1, 2, 3];

const b = a; // another reference to the same array
b[0] = 99;
console.log("b = a, then b[0] = 99: a is", show(a));

const c = [...a]; // spread the items into a new array
c[0] = 0;
console.log("c = copy of a, then c[0] = 0: a is", show(a));

setFirst(a);
console.log("set_first(a) sets a[0] = -1: a is", show(a));

const grid = [
  [0, 0],
  [0, 0],
];
const g = [...grid]; // a new outer array holding the same two rows
g[0][0] = 1;
console.log("g = copy of grid, then g[0][0] = 1: grid is", show(grid));
