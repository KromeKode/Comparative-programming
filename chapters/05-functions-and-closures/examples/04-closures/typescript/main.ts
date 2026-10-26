const makeAdder =
  (n: number) =>
  (x: number): number =>
    x + n;

function makeAccumulator(): (x: number) => number {
  let total = 0;
  return (x) => {
    total += x;
    return total;
  };
}

const add5 = makeAdder(5);
const add10 = makeAdder(10);
console.log(`add5(10) = ${add5(10)}`);
console.log(`add10(10) = ${add10(10)}`);

const running = makeAccumulator();
console.log(`running total: ${running(3)} ${running(5)} ${running(2)}`);

const madeWithLet: (() => number)[] = [];
for (let i = 0; i < 3; i++) {
  madeWithLet.push(() => i); // let: each pass gets a fresh i
}
console.log(`made in a loop with let: ${madeWithLet.map((f) => f()).join(" ")}`);

const madeWithVar: (() => number)[] = [];
for (var j = 0; j < 3; j++) {
  madeWithVar.push(() => j); // var: one j shared by every pass, 3 by the time we call
}
console.log(`made in a loop with var: ${madeWithVar.map((f) => f()).join(" ")}`);
export {};
