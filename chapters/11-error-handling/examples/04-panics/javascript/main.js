import assert from "node:assert";

const numbers = [10, 20, 30];
const users = new Map([[1, { name: "ada" }]]);
const [index, zero] = [5, 0];

// findUser returns undefined when there is no such user.
function findUser(id) {
  return users.get(id);
}

// Run action and report what it returned or which error it threw.
function attempt(label, action) {
  try {
    console.log(`${label}: ${action()}`);
  } catch (error) {
    console.log(`${label}: ${error.name}: ${error.message}`);
  }
}

// Only two of these throw. The other two give a value and carry on.
attempt("numbers[5]", () => numbers[index]);
attempt("10 / zero", () => 10 / zero);
attempt("findUser(7).name", () => findUser(7).name);
attempt("assert(numbers.length === 4)", () => {
  assert(numbers.length === 4, "expected 4 numbers");
  return "passed";
});
