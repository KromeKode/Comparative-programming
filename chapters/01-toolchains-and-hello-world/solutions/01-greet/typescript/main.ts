import { readFileSync } from "node:fs";

const firstLine: string = readFileSync(0, "utf8").split("\n")[0] ?? "";
const name: string = firstLine.trim();
console.log(name === "" ? "Hello, stranger!" : `Hello, ${name}!`);
