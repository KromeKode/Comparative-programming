function greet(name: string, greeting = "Hello", punctuation = "!"): string {
  return `${greeting}, ${name}${punctuation}`;
}

// ...values (a rest parameter) collects any number of arguments into an array.
function total(...values: number[]): number {
  let sum = 0;
  for (const value of values) {
    sum += value;
  }
  return sum;
}

console.log(greet("Ada"));
console.log(greet("Grace", "Hi"));
console.log(greet("Linus", undefined, "?")); // undefined means "use the default"

console.log(`total() = ${total()}`);
console.log(`total(1, 2, 3) = ${total(1, 2, 3)}`);
const numbers: number[] = [4, 3, 2, 1];
console.log(`total of [4, 3, 2, 1] = ${total(...numbers)}`); // ... spreads an array
export {};
