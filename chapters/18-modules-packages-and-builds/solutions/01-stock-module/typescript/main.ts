// Exercise 18.1: a stock module. This file is given: write stock.ts so that it
// runs. It reads the commands, and leaves keeping count to the module.
import { readFileSync } from "node:fs";

import { Stock } from "./stock.ts";

// An amount is a whole number from 1 to 999,999,999,999,999.
const AMOUNT = /^0*[1-9][0-9]{0,14}$/;

const stock = new Stock();
const lines = readFileSync(0, "utf8").split("\n");
if (lines.at(-1) === "") lines.pop();
lines.forEach((raw, index) => {
  const number = index + 1;
  const words = raw.split(/\s+/).filter((word) => word !== "");
  if (words.length === 0) return;
  const [command, item, amount] = words;
  const ok = words.length === 3 && AMOUNT.test(amount);
  if (ok && command === "add") {
    stock.add(item, Number(amount));
    console.log(`added ${Number(amount)} ${item}, now ${stock.count(item)}`);
  } else if (ok && command === "take") {
    if (stock.take(item, Number(amount))) {
      console.log(`took ${Number(amount)} ${item}, now ${stock.count(item)}`);
    } else {
      console.log(`error: line ${number}: only ${stock.count(item)} ${item} in stock`);
    }
  } else if (words.length === 2 && command === "count") {
    console.log(`${item}: ${stock.count(item)}`);
  } else if (words.length === 1 && command === "list") {
    const items = stock.items().map(([name, count]) => `${name} ${count}`);
    console.log(`stock: ${items.length > 0 ? items.join(", ") : "empty"}`);
  } else {
    console.log(`error: line ${number}: expected add, take, count, or list`);
  }
});
