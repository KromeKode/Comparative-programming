const READ = 0b001;
const WRITE = 0b010;
const EXEC = 0b100;

const bits8 = (n) => n.toString(2).padStart(8, "0");

function countOnes(n) {
  let count = 0;
  for (let rest = n; rest !== 0; rest >>>= 1) {
    count += rest & 1;
  }
  return count;
}

let perms = READ | WRITE; // turn bits on with |
console.log(`read | write: ${bits8(perms)}`);
// Test a bit with &. The parentheses matter: === binds tighter than & in JavaScript.
console.log(`has write? ${(perms & WRITE) !== 0 ? "yes" : "no"}`);
console.log(`has exec? ${(perms & EXEC) !== 0 ? "yes" : "no"}`);
perms &= ~WRITE; // turn a bit off: AND with every bit except WRITE
console.log(`clear write: ${bits8(perms)}`);
perms ^= EXEC; // flip a bit with ^
console.log(`toggle exec: ${bits8(perms)}`);
console.log(`5 << 2 = ${5 << 2}`);
console.log(`-16 >> 2 = ${-16 >> 2}`);
console.log(`set bits in 10110: ${countOnes(0b10110)}`);
