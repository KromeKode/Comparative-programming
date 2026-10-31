const show = (value: unknown): string =>
  JSON.stringify(value).replaceAll(",", ", ");
const yesNo = (flag: boolean): string => (flag ? "yes" : "no");

function addFive(items: number[]): void {
  items.push(5); // changes the caller's array, because items refers to it
}

// TypeScript has no built-in way to compare two arrays item by item.
function equalItems(x: readonly number[], y: readonly number[]): boolean {
  return x.length === y.length && x.every((item, i) => item === y[i]);
}

// Every object is reached through a reference. Assignment copies the reference.
// `const` only stops a from being pointed elsewhere; the array can still change.
const a: number[] = [1, 2, 3];
const b = a;
b.push(4);
console.log(`b = a, then b gets 4: a = ${show(a)}`);

addFive(a);
console.log(`a function appends 5: a = ${show(a)}`);

const n = 1;
let m = n;
m += 1;
console.log(`m = n, then m += 1: n = ${n}, m = ${m}`);

const grid: number[][] = [
  [1, 2],
  [3, 4],
];
const shallow = [...grid];
shallow[0][0] = 9;
console.log(`shallow copy, then copy[0][0] = 9: grid = ${show(grid)}`);

const deep = structuredClone(grid);
deep[0][0] = 7;
console.log(`deep copy, then copy[0][0] = 7: grid = ${show(grid)}`);

const rows: number[][] = Array(3).fill([0, 0]);
rows[0][0] = 1;
console.log(`one row in 3 slots, then rows[0][0] = 1: rows = ${show(rows)}`);

const same = [...a];
console.log(`a and b are the same list: ${yesNo(a === b)}`);
console.log(`a and its copy are the same list: ${yesNo(a === same)}`);
console.log(`a and its copy hold equal items: ${yesNo(equalItems(a, same))}`);
