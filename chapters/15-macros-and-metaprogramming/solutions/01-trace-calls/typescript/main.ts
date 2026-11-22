import { readFileSync } from "node:fs";

const tracer = { on: false, depth: 0, calls: 0 };

function say(text: string): void {
  if (tracer.on) console.log("  ".repeat(tracer.depth) + text);
}

type Fn = (...args: number[]) => number;

// traceAll gives back an object with the same methods, each one replaced by
// a wrapper that traces its calls. Each body runs with this set to the new
// object, so this.fib calls the wrapper, and the recursive calls are traced
// too. Inside the methods, the compiler types this as T.
function traceAll<T extends Record<string, Fn>>(methods: T): T {
  const traced: Record<string, Fn> = {};
  for (const [name, body] of Object.entries(methods)) {
    const wrapper = (...args: number[]): number => {
      tracer.calls++;
      say(`${name}(${args.join(", ")})`);
      tracer.depth++;
      let result: number;
      try {
        result = body.apply(traced, args);
      } finally {
        tracer.depth--;
      }
      say(`-> ${result}`);
      return result;
    };
    // The wrapper takes ...args, so it is given the body's length (its number
    // of parameters) and name.
    Object.defineProperty(wrapper, "length", { value: body.length });
    Object.defineProperty(wrapper, "name", { value: name });
    traced[name] = wrapper;
  }
  // The types say what traced is, but the compiler can't check a loop that
  // builds it, so this cast is a promise.
  return traced as T;
}

const functions = traceAll({
  fib(n: number): number {
    return n < 2 ? n : this.fib(n - 1) + this.fib(n - 2);
  },

  gcd(a: number, b: number): number {
    return b === 0 ? a : this.gcd(b, a % b);
  },

  power(base: number, exponent: number): number {
    if (exponent === 0) return 1;
    if (exponent % 2 === 0) {
      const half = this.power(base, exponent / 2);
      return half * half;
    }
    return base * this.power(base, exponent - 1);
  },
});

const plural = (count: number, word: string): string =>
  `${count} ${word}${count === 1 ? "" : "s"}`;

function run(line: string): void {
  const [command, name, ...words] = line.trim().split(/\s+/);
  const args = words.map(Number);
  if (command !== "trace" && command !== "count") {
    console.log(`unknown command: ${command}`);
    return;
  }
  if (!Object.hasOwn(functions, name)) {
    console.log(`unknown function: ${name}`);
    return;
  }
  // The name came from the input, so the compiler only knows it is a string.
  const fn: Fn = functions[name as keyof typeof functions];
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
