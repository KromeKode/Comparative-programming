const show = (value: number | number[] | number[][]): string =>
  Array.isArray(value) ? `[${value.map(show).join(", ")}]` : `${value}`;

function setFirst(items: number[]): void {
  items[0] = -1;
}

const a = [1, 2, 3];

const b = a;
b[0] = 99;
console.log("b = a, then b[0] = 99: a is", show(a));

const c = [...a];
c[0] = 0;
console.log("c = copy of a, then c[0] = 0: a is", show(a));

setFirst(a);
console.log("set_first(a) sets a[0] = -1: a is", show(a));

// `const` stops reassigning grid, not changing it; readonly number[][] would stop both.
const grid = [
  [0, 0],
  [0, 0],
];
const g = [...grid];
g[0][0] = 1;
console.log("g = copy of grid, then g[0][0] = 1: grid is", show(grid));
