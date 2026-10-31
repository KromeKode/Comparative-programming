// console.log prints an array as [ 3, 1, 4 ], so this helper matches the other languages.
const show = (items) => `[${items.join(", ")}]`;

const numbers = [3, 1, 4, 1, 5];
console.log("numbers:", show(numbers));
console.log("length:", numbers.length);
console.log(`first: ${numbers[0]}, last: ${numbers.at(-1)}`); // at(-1) counts from the end

numbers.push(9);
console.log("append 9:", show(numbers));
numbers.splice(1, 0, 2); // at index 1, remove 0 items, insert 2
console.log("insert 2 at index 1:", show(numbers));
const removed = numbers.shift();
console.log(`remove index 0 (was ${removed}):`, show(numbers));

console.log("contains 4:", numbers.includes(4) ? "yes" : "no");
console.log("index of 1:", numbers.indexOf(1)); // -1 if it is missing

numbers.sort((a, b) => a - b); // without a comparator, sort compares as strings
console.log("sorted:", show(numbers));
console.log("slice 1..4:", show(numbers.slice(1, 4))); // a new array, copied

numbers.reverse();
console.log("reversed:", show(numbers));
