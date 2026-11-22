import { readFileSync } from "node:fs";

const tracer = { on: false, depth: 0, calls: 0 };

function say(text) {
  if (tracer.on) console.log("  ".repeat(tracer.depth) + text);
}

// traceAll gives back an object with the same methods, each one replaced by
// a wrapper that traces its calls. Object.entries lists the methods by name.
// Each body runs with this set to the new object, so this.fib(n - 1) calls
// the wrapper, and the recursive calls are traced too.
function traceAll(methods) {
  const traced = {};
  for (const [name, body] of Object.entries(methods)) {
    const wrapper = (...args) => {
      tracer.calls++;
      say(`${name}(${args.join(", ")})`);
      tracer.depth++;
      let result;
      try {
        result = body.apply(traced, args);
      } finally {
        tracer.depth--;
      }
      say(`-> ${result}`);
      return result;
    };
    // A function's length is the number of parameters it declares. The
    // wrapper takes ...args, so it is given the body's length and name.
    Object.defineProperty(wrapper, "length", { value: body.length });
    Object.defineProperty(wrapper, "name", { value: name });
    traced[name] = wrapper;
  }
  return traced;
}

const functions = traceAll({
  fib(n) {
    return n < 2 ? n : this.fib(n - 1) + this.fib(n - 2);
  },

  gcd(a, b) {
    return b === 0 ? a : this.gcd(b, a % b);
  },

  power(base, exponent) {
    if (exponent === 0) return 1;
    if (exponent % 2 === 0) {
      const half = this.power(base, exponent / 2);
      return half * half;
    }
    return base * this.power(base, exponent - 1);
  },
});

const plural = (count, word) => `${count} ${word}${count === 1 ? "" : "s"}`;

function run(line) {
  const [command, name, ...words] = line.trim().split(/\s+/);
  const args = words.map(Number);
  if (command !== "trace" && command !== "count") {
    console.log(`unknown command: ${command}`);
    return;
  }
  // Object.hasOwn skips names that every object inherits, such as toString.
  if (!Object.hasOwn(functions, name)) {
    console.log(`unknown function: ${name}`);
    return;
  }
  const fn = functions[name];
  if (args.length !== fn.length) {
    console.log(
      `${name} takes ${plural(fn.length, "argument")}, got ${args.length}`,
    );
    return;
  }
  Object.assign(tracer, { on: command === "trace", depth: 0, calls: 0 });
  const result = fn(...args);
  console.log(
    `${name}(${args.join(", ")}) = ${result} in ${plural(tracer.calls, "call")}`,
  );
}

for (const line of readFileSync(0, "utf8").split("\n")) {
  if (line.trim() !== "") run(line);
}
