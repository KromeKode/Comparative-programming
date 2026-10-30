const addOne = (x) => x + 1; // an arrow function stored in a variable
function triple(x) {
  return x * 3;
}

function applyTwice(f, x) {
  return f(f(x));
}

const compose = (outer, inner) => (x) => outer(inner(x));

console.log(`apply_twice(add_one, 5) = ${applyTwice(addOne, 5)}`);
console.log(`apply_twice(triple, 5) = ${applyTwice(triple, 5)}`);
console.log(`apply_twice(square, 3) = ${applyTwice((x) => x * x, 3)}`);
const addThenTriple = compose(triple, addOne);
console.log(`compose(triple, add_one)(5) = ${addThenTriple(5)}`);
