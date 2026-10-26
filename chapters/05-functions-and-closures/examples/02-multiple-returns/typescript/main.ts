// A tuple type says exactly how many values come back, and their types.
function minMax(values: number[]): [number, number] {
  let lowest = values[0];
  let highest = values[0];
  for (const value of values) {
    lowest = Math.min(lowest, value);
    highest = Math.max(highest, value);
  }
  return [lowest, highest];
}

function divMod(a: number, b: number): { quotient: number; remainder: number } {
  return { quotient: Math.trunc(a / b), remainder: a % b };
}

const [lowest, highest] = minMax([3, -1, 4, 1, 5]);
console.log(`min and max of [3, -1, 4, 1, 5]: ${lowest} ${highest}`);

const { quotient, remainder } = divMod(17, 5); // object destructuring, by name
console.log(`17 divided by 5: ${quotient} remainder ${remainder}`);

let first = "hello";
let second = "world";
[first, second] = [second, first];
console.log(`swapped: ${first} ${second}`);
export {};
