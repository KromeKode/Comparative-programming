// Reads a trace of function calls and prints a flat profile: for each function, how
// many calls it got, its total time, and its self time. See ../README.md.
import { readFileSync } from "node:fs";

class TraceError extends Error {}

const isTime = (s) => /^[0-9]{1,12}$/.test(s);
const isName = (s) => /^[!-~]+$/.test(s);

function profile(lines) {
  const functions = new Map();
  const stack = [];
  let now = 0;
  lines.forEach((line, index) => {
    const number = index + 1;
    const fields = line.split(/[ \t]+/).filter((field) => field !== "");
    if (fields.length === 0) return;
    const [timeText, event, name] = fields;
    if (
      fields.length !== 3 ||
      !isTime(timeText) ||
      !isName(name) ||
      !["enter", "exit"].includes(event)
    ) {
      throw new TraceError(
        `line ${number}: expected TIME enter NAME or TIME exit NAME`,
      );
    }
    const time = Number(timeText);
    if (time < now) {
      throw new TraceError(
        `line ${number}: time goes back from ${now} to ${time}`,
      );
    }
    now = time;

    if (event === "enter") {
      if (!functions.has(name)) {
        functions.set(name, {
          calls: 0,
          total: 0,
          self: 0,
          running: 0,
          outermostStart: 0,
        });
      }
      const f = functions.get(name);
      f.calls++;
      if (f.running === 0) f.outermostStart = time;
      f.running++;
      stack.push({ name, start: time, inCallees: 0 });
      return;
    }

    if (stack.length === 0) {
      throw new TraceError(
        `line ${number}: exit from ${name}, but nothing is running`,
      );
    }
    const call = stack.at(-1);
    if (call.name !== name) {
      throw new TraceError(
        `line ${number}: exit from ${name}, but the innermost call is to ${call.name}`,
      );
    }
    stack.pop();
    const duration = time - call.start;
    const f = functions.get(name);
    f.self += duration - call.inCallees;
    f.running--;
    // A recursive call's time is already in the total of the call around it.
    if (f.running === 0) f.total += time - f.outermostStart;
    if (stack.length > 0) stack.at(-1).inCallees += duration;
  });
  if (stack.length > 0) {
    throw new TraceError(`end of trace: ${stack.at(-1).name} is still running`);
  }
  return functions;
}

try {
  const functions = profile(readFileSync(0, "utf8").split("\n"));
  if (functions.size === 0) console.log("no calls");
  const rows = [...functions].sort(
    ([a, f], [b, g]) => g.self - f.self || (a < b ? -1 : a > b ? 1 : 0),
  );
  for (const [name, f] of rows) {
    console.log(`${name} ${f.calls} ${f.total} ${f.self}`);
  }
} catch (error) {
  if (!(error instanceof TraceError)) throw error;
  console.log(`error: ${error.message}`);
}
