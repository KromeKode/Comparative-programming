import assert from "node:assert";

interface User {
  name: string;
}

const numbers: number[] = [10, 20, 30];
const users = new Map<number, User>([[1, { name: "ada" }]]);
const [index, zero] = [5, 0];

function findUser(id: number): User | undefined {
  return users.get(id);
}

// Run action and report what it returned or which error it threw.
function attempt(label: string, action: () => unknown): void {
  try {
    console.log(`${label}: ${action()}`);
  } catch (error) {
    if (!(error instanceof Error)) {
      throw error;
    }
    console.log(`${label}: ${error.name}: ${error.message}`);
  }
}

// numbers[index] has the type number, even though it gives undefined here.
attempt("numbers[5]", () => numbers[index]);
attempt("10 / zero", () => 10 / zero);
// findUser(7).name does not compile: the result is possibly undefined.
// The ! tells the compiler "trust me", so it compiles, and fails when run.
attempt("findUser(7)!.name", () => findUser(7)!.name);
attempt("assert(numbers.length === 4)", () => {
  assert(numbers.length === 4, "expected 4 numbers");
  return "passed";
});
