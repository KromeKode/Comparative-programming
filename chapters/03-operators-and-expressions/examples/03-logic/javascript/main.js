let calls = 0; // counts how many times the right side ran

function bigRatio(x) {
  calls++;
  return 10 / x > 1; // 10 / 0 does not crash: it is Infinity, a silently wrong answer
}

function displayName(name) {
  // || returns its left operand if that is truthy, and its right operand otherwise.
  return name || "stranger";
}

for (const x of [0, 5]) {
  // && stops as soon as the left side is falsy, so bigRatio never sees 0.
  const ok = x !== 0 && bigRatio(x);
  console.log(`x = ${x}: ${ok ? "yes" : "no"}`);
}
console.log(`right side ran ${calls} of 2 times`);
console.log(`name "": ${displayName("")}`);
console.log(`name "Ada": ${displayName("Ada")}`);
