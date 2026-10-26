// JavaScript has no pattern matching. Destructuring unpacks the pair,
// and an if chain does the rest.
function classify([x, y]) {
  if (x === 0 && y === 0) {
    return "origin";
  }
  if (y === 0) {
    return `on the x axis at x = ${x}`;
  }
  if (x === 0) {
    return `on the y axis at y = ${y}`;
  }
  if (x === y) {
    return `on the diagonal at ${x}`;
  }
  return `x = ${x}, y = ${y}`;
}

const points = [
  [0, 0],
  [3, 0],
  [0, -2],
  [4, 4],
  [-1, 5],
];
for (const [x, y] of points) {
  console.log(`(${x}, ${y}): ${classify([x, y])}`);
}
