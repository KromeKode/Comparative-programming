const fruits = ["apple", "banana", "cherry"];
console.log("joined:", fruits.join(", "));

// Strings are immutable, but engines make += in a loop fast, so this is fine.
let built = "";
for (let n = 1; n <= 5; n++) {
  built += n > 1 ? `-${n}` : `${n}`;
}
console.log("built:", built);

console.log(`padded: [${"Ada".padEnd(8)}] [${String(90).padStart(8)}]`);
console.log(`zero-padded: ${String(42).padStart(5, "0")}`);
console.log(`two decimals: ${(3.14159).toFixed(2)}`);
