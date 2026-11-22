import { readFileSync } from "node:fs";

// A discriminated union: checking kind tells TypeScript which other property
// the step has.
type Step =
  | { kind: "print"; text: string }
  | { kind: "sleep"; duration: number }
  | { kind: "wait"; task: number }; // the number of the task to wait for

interface Task {
  name: string;
  steps: Step[];
  next: number; // the index of the step to run when it runs again
  done: boolean;
  waitingFor: number;
  waiters: number[]; // in the order they began waiting
}

interface Sleeper {
  wake: number;
  order: number;
  task: number;
}

// The first word of a line, and the rest of it.
function split(line: string): [string, string] {
  const trimmed = line.trim();
  const space = trimmed.indexOf(" ");
  if (space < 0) return [trimmed, ""];
  return [trimmed.slice(0, space), trimmed.slice(space).trim()];
}

// Returns the tasks, and prints an error for each line with a problem.
function parse(lines: string[]): Task[] {
  // A wait can name a task defined further down, so the names come first.
  const numbers = new Map<string, number>();
  for (const line of lines) {
    const [command, name] = split(line);
    if (command === "task" && !numbers.has(name))
      numbers.set(name, numbers.size);
  }
  const tasks: Task[] = [];
  for (const [i, line] of lines.entries()) {
    const [command, rest] = split(line);
    const error = (message: string) =>
      console.log(`error: line ${i + 1}: ${message}`);
    const steps = tasks.at(-1)?.steps;
    const other = numbers.get(rest);
    if (command === "") continue;
    if (command === "task") {
      tasks.push({
        name: rest,
        steps: [],
        next: 0,
        done: false,
        waitingFor: 0,
        waiters: [],
      });
    } else if (steps === undefined) {
      error("a step before any task");
    } else if (command === "print") {
      steps.push({ kind: "print", text: rest });
    } else if (command === "sleep") {
      if (/^[0-9]+$/.test(rest))
        steps.push({ kind: "sleep", duration: Number(rest) });
      else error("sleep takes a whole number");
    } else if (command === "wait") {
      if (other !== undefined) steps.push({ kind: "wait", task: other });
      else error(`no task named ${rest}`);
    } else {
      error(`unknown step: ${command}`);
    }
  }
  return tasks;
}

class Loop {
  time = 0;
  readonly tasks: Task[];
  readonly ready: number[]; // the tasks that can run now, in order
  // TypeScript has no heap, so the loop searches this for the task to wake
  // first.
  readonly sleeping: Sleeper[] = [];
  sleeps = 0;

  constructor(tasks: Task[]) {
    this.tasks = tasks;
    this.ready = tasks.map((_, id) => id);
  }

  // Runs a task until it sleeps, waits, or ends.
  runUntilStopped(id: number): void {
    const task = this.tasks[id];
    while (task.next < task.steps.length) {
      const step = task.steps[task.next++];
      switch (step.kind) {
        case "print":
          console.log(`[${this.time}] ${task.name}: ${step.text}`);
          break;
        case "sleep":
          if (step.duration === 0) this.ready.push(id);
          else
            this.sleeping.push({
              wake: this.time + step.duration,
              order: this.sleeps++,
              task: id,
            });
          return;
        case "wait":
          if (this.tasks[step.task].done) break; // it has finished already, so carry on
          task.waitingFor = step.task;
          this.tasks[step.task].waiters.push(id);
          return;
      }
    }
    task.done = true;
    console.log(`[${this.time}] ${task.name}: done`);
    this.ready.push(...task.waiters);
  }

  firstToWake(): number {
    let best = 0;
    for (const [i, { wake, order }] of this.sleeping.entries()) {
      const first = this.sleeping[best];
      if (wake < first.wake || (wake === first.wake && order < first.order))
        best = i;
    }
    return best;
  }

  run(): void {
    for (;;) {
      for (
        let id = this.ready.shift();
        id !== undefined;
        id = this.ready.shift()
      ) {
        this.runUntilStopped(id);
      }
      if (this.sleeping.length === 0) break;
      // Nothing can run now, so the clock jumps to the next wake.
      this.time = this.sleeping[this.firstToWake()].wake;
      while (
        this.sleeping.length > 0 &&
        this.sleeping[this.firstToWake()].wake === this.time
      ) {
        this.ready.push(this.sleeping.splice(this.firstToWake(), 1)[0].task);
      }
    }
    const stuck = this.tasks
      .filter((task) => !task.done)
      .map(
        (task) => `${task.name} waits for ${this.tasks[task.waitingFor].name}`,
      );
    if (stuck.length > 0)
      console.log(`[${this.time}] deadlock: ${stuck.join(", ")}`);
    else console.log(`[${this.time}] all done`);
  }
}

const lines = readFileSync(0, "utf8").split("\n");
if (lines.at(-1) === "") lines.pop(); // after the newline that ends the input
new Loop(parse(lines)).run();
