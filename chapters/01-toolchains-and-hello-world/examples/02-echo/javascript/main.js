import { readFileSync } from "node:fs";

const input = readFileSync(0, "utf8"); // 0 is stdin's file descriptor
const line = input.split(/\r?\n/)[0];
console.log(`You typed: ${line}`);
