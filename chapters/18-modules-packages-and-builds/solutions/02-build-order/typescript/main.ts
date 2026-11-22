// Exercise 18.2: build order. The order to build modules in, and what to rebuild.
import { readFileSync } from "node:fs";

const NAME = /^[A-Za-z0-9_./-]+$/;

// Compares strings by their UTF-16 code units, which for ASCII is alphabetical
// order with capitals first.
const byName = (a: string, b: string): number => (a < b ? -1 : a > b ? 1 : 0);

interface Module {
  line: number;
  imports: string[]; // as written, with repeats dropped
}

function run(lines: string[]): string[] {
  const out: string[] = [];
  // A Map keeps its keys in the order they were added: the order of definition.
  const modules = new Map<string, Module>();
  const queries: [number, string][] = [];
  for (const [index, line] of lines.entries()) {
    const number = index + 1;
    const colon = line.indexOf(":");
    if (colon >= 0) {
      const name = line.slice(0, colon).trim();
      const imports = line.slice(colon + 1).split(/\s+/).filter((word) => word !== "");
      if (!NAME.test(name) || !imports.every((word) => NAME.test(word))) {
        out.push(`error: line ${number}: a module is NAME: IMPORTS`);
        return out;
      }
      const earlier = modules.get(name);
      if (earlier !== undefined) {
        out.push(`error: line ${number}: ${name} is already defined on line ${earlier.line}`);
        return out;
      }
      // A Set drops repeats and keeps the first of each, in order.
      modules.set(name, { line: number, imports: [...new Set(imports)] });
      continue;
    }
    const words = line.split(/\s+/).filter((word) => word !== "");
    if (words.length === 0) continue;
    if (words.length === 2 && words[0] === "rebuild") {
      queries.push([number, words[1]]);
      continue;
    }
    out.push(`error: line ${number}: expected NAME: IMPORTS or rebuild NAME`);
    return out;
  }

  const users = new Map<string, string[]>([...modules.keys()].map((name) => [name, []]));
  const waiting = new Map<string, number>();
  for (const [name, { line, imports }] of modules) {
    for (const imported of imports) {
      if (!modules.has(imported)) {
        out.push(`error: line ${line}: ${name} imports ${imported}, which isn't defined`);
        return out;
      }
      users.get(imported)!.push(name);
    }
    waiting.set(name, imports.length);
  }

  // Kahn's algorithm: a module is ready once everything it imports is built, and
  // of the ready modules, the first in alphabetical order goes next. JavaScript has
  // no heap, so this looks through the ready modules for the first.
  const ready = [...modules.keys()].filter((name) => waiting.get(name) === 0);
  const order: string[] = [];
  while (ready.length > 0) {
    const first = ready.reduce((best, name, i) => (byName(name, ready[best]) < 0 ? i : best), 0);
    const [name] = ready.splice(first, 1);
    order.push(name);
    for (const user of users.get(name)!) {
      waiting.set(user, waiting.get(user)! - 1);
      if (waiting.get(user) === 0) ready.push(user);
    }
  }

  if (order.length < modules.size) {
    // Every module left over imports another one left over, so following first
    // imports from the first module left over must come back to a module.
    const isLeft = (name: string): boolean => waiting.get(name)! > 0;
    const path = [[...modules.keys()].filter(isLeft).sort(byName)[0]];
    const seen = new Map([[path[0], 0]]);
    for (;;) {
      const next = modules.get(path.at(-1)!)!.imports.find(isLeft)!;
      if (seen.has(next)) {
        const cycle = [...path.slice(seen.get(next)), next];
        out.push(`error: import cycle: ${cycle.join(" -> ")}`);
        return out;
      }
      seen.set(next, path.length);
      path.push(next);
    }
  }

  out.push(`build order: ${order.length > 0 ? order.join(", ") : "nothing to build"}`);
  for (const [number, name] of queries) {
    if (!modules.has(name)) {
      out.push(`error: line ${number}: no module called ${name}`);
      continue;
    }
    // Everything that imports the changed module, directly or not, comes after it
    // in the build order, so one pass through the order finds them all.
    const affected = new Set([name]);
    for (const module of order) {
      if (modules.get(module)!.imports.some((imported) => affected.has(imported))) {
        affected.add(module);
      }
    }
    // A Set keeps the order things were added in, which here is build order.
    out.push(`rebuild ${name}: ${[...affected].join(", ")}`);
  }
  return out;
}

const lines = readFileSync(0, "utf8").split("\n");
if (lines.at(-1) === "") lines.pop();
for (const line of run(lines)) console.log(line);
