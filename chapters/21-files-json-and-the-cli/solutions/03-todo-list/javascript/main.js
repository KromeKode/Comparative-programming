// Keeps a to-do list in todo.json, in the current folder: adds tasks, lists them, marks
// them done, and removes them. ../README.md says what it should do.
import { readFileSync, renameSync, rmSync, writeFileSync } from "node:fs";

const FILE = "todo.json";
const VERSION = 1;
const USAGE = `usage: todo add TEXT...
       todo list
       todo done ID
       todo remove ID`;
// Node's errors have the C names of the system's error codes, such as ENOENT, and
// messages of their own; these are the messages strerror gives for them.
const MESSAGES = { ENOENT: "No such file or directory", EISDIR: "Is a directory" };

// A problem with todo.json, which stops the program with status 2.
class Problem extends Error {}

// typeof says "object" for null and for arrays too.
function isObject(value) {
  return value !== null && typeof value === "object" && !Array.isArray(value);
}

function taskFromJson(value) {
  // The keys, sorted, show that the object has these three and no others.
  if (
    isObject(value) &&
    Object.keys(value).sort().join() === "done,id,text" &&
    Number.isInteger(value.id) &&
    value.id >= 1 &&
    typeof value.text === "string" &&
    typeof value.done === "boolean"
  ) {
    return { id: value.id, text: value.text, done: value.done };
  }
  throw new Problem("not a to-do list");
}

// The tasks in todo.json, or none if there's no todo.json.
function readTasks() {
  let text;
  try {
    text = readFileSync(FILE, "utf8");
  } catch (error) {
    if (error.code === "ENOENT") return [];
    throw new Problem(MESSAGES[error.code] ?? error.message);
  }
  let value;
  try {
    value = JSON.parse(text);
  } catch {
    throw new Problem("not valid JSON");
  }
  if (!isObject(value)) throw new Problem("not a to-do list");
  // A later version of todo may keep anything else in the file, so the version comes
  // first.
  if (typeof value.version === "number" && value.version !== VERSION) {
    throw new Problem(`unknown version ${value.version}`);
  }
  if (value.version !== VERSION || Object.keys(value).length !== 2 || !Array.isArray(value.tasks)) {
    throw new Problem("not a to-do list");
  }
  const tasks = value.tasks.map(taskFromJson);
  if (new Set(tasks.map((task) => task.id)).size !== tasks.length) {
    throw new Problem("not a to-do list"); // two tasks with one id
  }
  return tasks;
}

// Writes the tasks to todo.json. It writes a new file and renames it over the old one,
// which replaces it in one step, so that a crash leaves the old list or the new one, and
// never half of one.
function writeTasks(tasks) {
  // JSON.stringify writes an object's keys in the order they were added.
  const text = JSON.stringify({ version: VERSION, tasks }, null, 2) + "\n";
  const temporary = `${FILE}.tmp`;
  try {
    writeFileSync(temporary, text, { flush: true }); // flush: onto the disk before the rename
    renameSync(temporary, FILE);
  } catch (error) {
    rmSync(temporary, { force: true });
    throw new Problem(MESSAGES[error.code] ?? error.message);
  }
}

// Carries out a command whose arguments have been checked, and returns the status.
function run(command, args) {
  const tasks = readTasks();
  if (command === "add") {
    const added = { id: Math.max(0, ...tasks.map((task) => task.id)) + 1, text: args.join(" "), done: false };
    tasks.push(added);
    writeTasks(tasks);
    console.log(`added ${added.id}: ${added.text}`);
    return 0;
  }
  if (command === "list") {
    for (const task of tasks) {
      console.log(`${task.id} [${task.done ? "x" : " "}] ${task.text}`);
    }
    return 0;
  }
  const id = Number(args[0]);
  const found = tasks.find((task) => task.id === id);
  if (found === undefined) {
    console.error(`todo: no task ${args[0]}`);
    return 1;
  }
  if (command === "done") {
    found.done = true;
    writeTasks(tasks);
    console.log(`done ${found.id}: ${found.text}`);
  } else {
    tasks.splice(tasks.indexOf(found), 1);
    writeTasks(tasks);
    console.log(`removed ${found.id}: ${found.text}`);
  }
  return 0;
}

function main(argv) {
  const [command = "", ...args] = argv;
  // The command line is checked before todo.json is read. An ID is digits: Number()
  // would take " 1", "1e3", and "0x10" too.
  const ok =
    (command === "add" && args.length > 0) ||
    (command === "list" && args.length === 0) ||
    ((command === "done" || command === "remove") && args.length === 1 && /^[0-9]+$/.test(args[0]));
  if (!ok) {
    console.error(USAGE);
    return 2;
  }
  try {
    return run(command, args);
  } catch (error) {
    if (!(error instanceof Problem)) throw error;
    console.error(`todo: ${FILE}: ${error.message}`);
    return 2;
  }
}

process.exitCode = main(process.argv.slice(2));
