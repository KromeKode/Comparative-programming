import { createInterface } from "node:readline";

class EditError extends Error {}

function checkLine(lines, number) {
  if (!(number >= 1 && number <= lines.length)) {
    throw new EditError(`no line ${number}`);
  }
}

// Each command is an edit that knows how to do itself and how to undo itself.
// Any object with apply, undo, and describe methods will do.
class Add {
  #text;

  constructor(text) {
    this.#text = text;
  }

  apply(lines) {
    lines.push(this.#text);
  }

  undo(lines) {
    lines.pop();
  }

  describe() {
    return `add "${this.#text}"`;
  }
}

class Delete {
  #number;
  #removed = "";

  constructor(number) {
    this.#number = number;
  }

  apply(lines) {
    checkLine(lines, this.#number);
    [this.#removed] = lines.splice(this.#number - 1, 1);
  }

  undo(lines) {
    lines.splice(this.#number - 1, 0, this.#removed);
  }

  describe() {
    return `delete line ${this.#number}`;
  }
}

class Replace {
  #number;
  #text;
  #old = "";

  constructor(number, text) {
    this.#number = number;
    this.#text = text;
  }

  apply(lines) {
    checkLine(lines, this.#number);
    this.#old = lines[this.#number - 1];
    lines[this.#number - 1] = this.#text;
  }

  undo(lines) {
    lines[this.#number - 1] = this.#old;
  }

  describe() {
    return `replace line ${this.#number} with "${this.#text}"`;
  }
}

class Editor {
  #lines = [];
  #done = [];
  #undone = [];

  run(command) {
    command.apply(this.#lines); // if this throws, nothing is recorded
    this.#done.push(command);
    this.#undone = [];
  }

  undo() {
    const command = this.#done.pop();
    if (command === undefined) {
      throw new EditError("nothing to undo");
    }
    command.undo(this.#lines);
    this.#undone.push(command);
    console.log(`undo: ${command.describe()}`);
  }

  redo() {
    const command = this.#undone.pop();
    if (command === undefined) {
      throw new EditError("nothing to redo");
    }
    command.apply(this.#lines);
    this.#done.push(command);
    console.log(`redo: ${command.describe()}`);
  }

  show() {
    if (this.#lines.length === 0) {
      console.log("(empty)");
    }
    this.#lines.forEach((line, i) => console.log(`${i + 1}: ${line}`));
  }
}

function run(editor, words) {
  const [command, ...args] = words;
  if (command === "add") {
    editor.run(new Add(args.join(" ")));
  } else if (command === "delete" && args.length === 1) {
    editor.run(new Delete(Number(args[0])));
  } else if (command === "replace" && args.length >= 1) {
    editor.run(new Replace(Number(args[0]), args.slice(1).join(" ")));
  } else if (command === "undo") {
    editor.undo();
  } else if (command === "redo") {
    editor.redo();
  } else if (command === "show") {
    editor.show();
  }
}

const editor = new Editor();
for await (const line of createInterface({ input: process.stdin })) {
  const words = line.split(/\s+/).filter((word) => word !== "");
  try {
    run(editor, words);
  } catch (error) {
    if (!(error instanceof EditError)) {
      throw error;
    }
    console.log(`error: ${error.message}`);
  }
}
