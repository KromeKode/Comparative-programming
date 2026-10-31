// A Map remembers the order its keys were added in.
const show = (stock) =>
  [...stock].map(([key, value]) => `${key}=${value}`).join(" ");

const stock = new Map([
  ["apple", 3],
  ["banana", 5],
]);
console.log("stock:", show(stock));

stock.set("cherry", 7);
console.log("add cherry:", show(stock));

console.log("banana:", stock.get("banana"));
if (!stock.has("durian")) {
  console.log(`durian: ${stock.get("durian") ?? 0} (missing)`); // get gives undefined
}

stock.set("apple", stock.get("apple") + 2);
console.log("apple += 2:", show(stock));
stock.delete("banana");
console.log("remove banana:", show(stock));

console.log("has cherry:", stock.has("cherry") ? "yes" : "no");
console.log("size:", stock.size);
