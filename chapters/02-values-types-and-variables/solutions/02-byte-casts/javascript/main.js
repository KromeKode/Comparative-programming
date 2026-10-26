import { readFileSync } from "node:fs";

for (const line of readFileSync(0, "utf8").split("\n")) {
  if (line.trim() === "") {
    continue;
  }
  const x = Number(line);
  const u8 = x & 0xff; // bitwise operators first convert to a 32-bit int
  const i8 = (x << 24) >> 24; // shift the low byte to the top, then back with the sign
  console.log(`${x} -> u8 ${u8}, i8 ${i8}`);
}
