import { readFileSync } from "node:fs";

const firstLine = readFileSync(0, "utf8").split("\n")[0];
const name = firstLine.trim();
console.log(name === "" ? "Hello, stranger!" : `Hello, ${name}!`);
