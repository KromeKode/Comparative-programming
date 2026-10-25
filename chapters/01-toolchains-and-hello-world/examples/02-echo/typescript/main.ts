import { readFileSync } from "node:fs";

const input: string = readFileSync(0, "utf8"); // 0 is stdin's file descriptor
const line: string = input.split(/\r?\n/)[0] ?? "";
console.log(`You typed: ${line}`);
