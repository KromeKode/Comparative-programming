const yesNo = (condition: boolean): string => (condition ? "yes" : "no");

const a: string = "hello";
const b: string = ["hel", "lo"].join(""); // built separately, same characters
console.log("strings with ===:", yesNo(a === b));

// Arrays are objects, and === on objects asks whether they are the same object.
const xs: number[] = [1, 2, 3];
const ys: number[] = [1, 2, 3];
console.log("arrays with ===:", yesNo(xs === ys));
const sameItems = xs.length === ys.length && xs.every((x, i) => x === ys[i]);
console.log("arrays element by element:", yesNo(sameItems));

const nan: number = 0 / 0;
console.log("NaN === NaN:", yesNo(nan === nan));
console.log("Number.isNaN:", yesNo(Number.isNaN(nan)));
export {};
