const yesNo = (condition) => (condition ? "yes" : "no");

const a = "hello";
const b = ["hel", "lo"].join(""); // built separately, same characters
console.log("strings with ===:", yesNo(a === b));

// Arrays are objects, and === on objects asks whether they are the same object.
const xs = [1, 2, 3];
const ys = [1, 2, 3];
console.log("arrays with ===:", yesNo(xs === ys));
const sameItems = xs.length === ys.length && xs.every((x, i) => x === ys[i]);
console.log("arrays element by element:", yesNo(sameItems));

const nan = 0 / 0;
console.log("NaN === NaN:", yesNo(nan === nan));
console.log("Number.isNaN:", yesNo(Number.isNaN(nan)));
