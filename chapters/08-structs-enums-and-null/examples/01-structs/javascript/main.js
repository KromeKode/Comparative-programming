// A plain object is JavaScript's struct. A variable holds a reference to it.
const show = (p) => `(${p.x}, ${p.y})`;
const stepsFromOrigin = (p) => Math.abs(p.x) + Math.abs(p.y);

const p = { x: 1, y: 2 };
console.log(`p = ${show(p)}`);
console.log(`p.x + p.y = ${p.x + p.y}`);

// Spread copies p's fields into a new object, and x: 5 replaces one of them.
const q = { ...p, x: 5 };
console.log(`q = p with x = 5: q = ${show(q)}, p = ${show(p)}`);

// Not a copy: r and p refer to the same object.
const r = p;
r.y = 9;
console.log(`r = p, then r.y = 9: r = ${show(r)}, p = ${show(p)}`);

// === on objects asks whether they are the same object, not whether fields match.
const a = { x: 3, y: -4 };
const b = { x: 3, y: -4 };
const same = a === b ? "yes" : "no";
console.log(`a = ${show(a)}, b = ${show(b)}, a == b: ${same}`);
console.log(`steps from (0, 0) to a: ${stepsFromOrigin(a)}`);
