// function* declares a generator function. Calling it runs none of the body:
// it returns a generator, and each next() runs the body to the next yield,
// where it pauses with all its variables kept.
function* fibonacci() {
  let [a, b] = [0, 1];
  for (;;) {
    yield a;
    [a, b] = [b, a + b];
  }
}

// yield* hands on everything another generator yields. Each recursive call
// pauses inside its own frame, so the generator keeps the path down the tree.
function* inOrder(node) {
  if (node === null) return;
  yield* inOrder(node.left);
  yield node.value;
  yield* inOrder(node.right);
}

const leaf = (value) => ({ value, left: null, right: null });
const tree = {
  value: 4,
  left: { value: 2, left: leaf(1), right: leaf(3) },
  right: { value: 6, left: leaf(5), right: leaf(7) },
};

console.log(`fibonacci: ${fibonacci().take(10).toArray().join(" ")}`);
console.log(`in order: ${[...inOrder(tree)].join(" ")}`);

// break stops asking at the first value over 3, so 5, 6, and 7 are never
// visited. There is no takeWhile helper, so the loop does it.
const upTo3 = [];
for (const value of inOrder(tree)) {
  if (value > 3) break;
  upTo3.push(value);
}
console.log(`up to 3: ${upTo3.join(" ")}`);
