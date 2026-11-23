// Reads a trace of function calls and prints a flat profile: for each function, how
// many calls it got, its total time, and its self time. See ../README.md.
import { readFileSync } from "node:fs";

interface FunctionStats {
  calls: number;
  total: number;
  self: number;
  running: number; // how many of its calls are running now
  outermostStart: number; // when the outermost of them started
}

interface Call {
  name: string;
  start: number;
  inCallees: number; // time spent in the functions it called
}

class TraceError extends Error {}

const isTime = (s: string): boolean => /^[0-9]{1,12}$/.test(s);
const isName = (s: string): boolean => /^[!-~]+$/.test(s);

function profile(lines: string[]): Map<string, FunctionStats> {
  const functions = new Map<string, FunctionStats>();
  const stack: Call[] = [];
  let now = 0;
  lines.forEach((line, index) => {
    const number = index + 1;
    const fields = line.split(/[ \t]+/).filter((field) => field !== "");
    if (fields.length === 0) return;
    const [timeText = "", event = "", name = ""] = fields;
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
      let f = functions.get(name);
      if (f === undefined) {
        f = { calls: 0, total: 0, self: 0, running: 0, outermostStart: 0 };
        functions.set(name, f);
      }
      f.calls++;
      if (f.running === 0) f.outermostStart = time;
      f.running++;
      stack.push({ name, start: time, inCallees: 0 });
      return;
    }

    const call = stack.at(-1);
    if (call === undefined) {
      throw new TraceError(
        `line ${number}: exit from ${name}, but nothing is running`,
      );
    }
    if (call.name !== name) {
      throw new TraceError(
        `line ${number}: exit from ${name}, but the innermost call is to ${call.name}`,
      );
    }
    stack.pop();
    const duration = time - call.start;
    const f = functions.get(name)!;
    f.self += duration - call.inCallees;
    f.running--;
    // A recursive call's time is already in the total of the call around it.
    if (f.running === 0) f.total += time - f.outermostStart;
    const caller = stack.at(-1);
    if (caller !== undefined) caller.inCallees += duration;
  });
  const innermost = stack.at(-1);
  if (innermost !== undefined) {
    throw new TraceError(`end of trace: ${innermost.name} is still running`);
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
