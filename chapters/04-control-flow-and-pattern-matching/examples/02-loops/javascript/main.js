let line = "up:";
for (let i = 0; i < 5; i++) {
  line += ` ${i}`;
}
console.log(line);

line = "down:";
for (let i = 5; i >= 1; i--) {
  line += ` ${i}`;
}
console.log(line);

line = "by threes:";
for (let i = 0; i < 10; i += 3) {
  line += ` ${i}`;
}
console.log(line);

line = "halving:";
let n = 100;
while (n > 0) {
  line += ` ${n}`;
  n = Math.trunc(n / 2);
}
console.log(line);

const colors = ["red", "green", "blue"];
line = "colors:";
for (const color of colors) {
  // for...of gives the values. for...in would give the keys "0", "1", "2".
  line += ` ${color}`;
}
console.log(line);

line = "numbered:";
for (const [i, color] of colors.entries()) {
  line += ` ${i + 1}=${color}`;
}
console.log(line);

// do-while tests the condition after the body, so the body always runs once.
let m = 100;
do {
  m *= 2;
} while (m < 50);
console.log(`do-while: ${m}`);
