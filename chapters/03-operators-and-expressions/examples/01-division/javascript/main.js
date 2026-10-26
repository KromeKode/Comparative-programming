// There is no integer type, so / always gives the exact quotient.
// Math.trunc drops the fraction, which matches C's integer division.
console.log("Math.trunc(7 / 2) =", Math.trunc(7 / 2));
console.log("Math.trunc(-7 / 2) =", Math.trunc(-7 / 2));
// % truncates like C: the remainder takes the sign of the left operand.
console.log("7 % 3 =", 7 % 3);
console.log("-7 % 3 =", -7 % 3);
console.log("7 % -3 =", 7 % -3);
console.log("7 / 2 =", 7 / 2);
// Math.floor rounds down instead, which matches Python's //.
console.log("Math.floor(-7 / 2) =", Math.floor(-7 / 2));
