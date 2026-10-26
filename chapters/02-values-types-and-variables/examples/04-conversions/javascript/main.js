const parsed = Number("42");
console.log(`"42" + 1 as a number: ${parsed + 1}`); // "42" + 1 without Number() is "421"

const x = 3.99;
console.log(`3.99 truncated: ${Math.trunc(x)}`);
console.log(`-3.99 truncated: ${Math.trunc(-x)}`);
console.log(`2.5 rounded: ${Math.round(2.5)}`); // halfway rounds up, toward +infinity
console.log(`-2.5 rounded: ${Math.round(-2.5)}`); // so this gives -2

console.log(`7 / 2 as integers: ${Math.trunc(7 / 2)}`); // there is no integer division
console.log(`7 / 2 as floats: ${7 / 2}`);
