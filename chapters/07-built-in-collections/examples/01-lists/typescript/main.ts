const show = (items: readonly number[]): string => `[${items.join(", ")}]`;

const numbers: number[] = [3, 1, 4, 1, 5];
console.log("numbers:", show(numbers));
console.log("length:", numbers.length);
// at() returns number | undefined, because the array might be empty.
console.log(`first: ${numbers[0]}, last: ${numbers.at(-1)}`);

numbers.push(9);
console.log("append 9:", show(numbers));
numbers.splice(1, 0, 2);
console.log("insert 2 at index 1:", show(numbers));
const removed = numbers.shift();
console.log(`remove index 0 (was ${removed}):`, show(numbers));

console.log("contains 4:", numbers.includes(4) ? "yes" : "no");
console.log("index of 1:", numbers.indexOf(1));

numbers.sort((a, b) => a - b);
console.log("sorted:", show(numbers));
console.log("slice 1..4:", show(numbers.slice(1, 4)));

numbers.reverse();
console.log("reversed:", show(numbers));
