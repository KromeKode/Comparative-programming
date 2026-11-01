import { createInterface } from "node:readline";

class EditError extends Error {}

// An edit that knows how to do itself and how to undo itself.
interface Command {
  apply(lines: string[]): void;
  undo(lines: string[]): void;
  describe(): string;
}

function checkLine(lines: readonly string[], number: number): void {
  if (!(number >= 1 && number <= lines.length)) {
    throw new EditError(`no line ${number}`);
  }
}

class Add implements Command {
  readonly #text: string;

  constructor(text: string) {
    this.#text = text;
  }

  apply(lines: string[]): void {
    lines.push(this.#text);
  }

  undo(lines: string[]): void {
    lines.pop();
  }

  describe(): string {
    return `add "${this.#text}"`;
  }
}

class Delete implements Command {
  readonly #number: number;
  #removed = "";

  constructor(number: number) {
    this.#number = number;
  }

  apply(lines: string[]): void {
    checkLine(lines, this.#number);
    this.#removed = lines.splice(this.#number - 1, 1)[0]!;
  }

  undo(lines: string[]): void {
    lines.splice(this.#number - 1, 0, this.#removed);
  }

  describe(): string {
    return `delete line ${this.#number}`;
  }
}

class Replace implements Command {
  readonly #number: number;
  readonly #text: string;
  #old = "";

  constructor(number: number, text: string) {
    this.#number = number;
    this.#text = text;
  }

  apply(lines: string[]): void {
    checkLine(lines, this.#number);
    this.#old = lines[this.#number - 1]!;
    lines[this.#number - 1] = this.#text;
  }

  undo(lines: string[]): void {
    lines[this.#number - 1] = this.#old;
  }

  describe(): string {
    return `replace line ${this.#number} with "${this.#text}"`;
  }
}

class Editor {
  readonly #lines: string[] = [];
  readonly #done: Command[] = [];
  #undone: Command[] = [];

  run(command: Command): void {
    command.apply(this.#lines); // if this throws, nothing is recorded
    this.#done.push(command);
    this.#undone = [];
  }

  undo(): void {
    const command = this.#done.pop();
    if (command === undefined) {
      throw new EditError("nothing to undo");
    }
    command.undo(this.#lines);
    this.#undone.push(command);
    console.log(`undo: ${command.describe()}`);
  }

  redo(): void {
    const command = this.#undone.pop();
    if (command === undefined) {
      throw new EditError("nothing to redo");
    }
    command.apply(this.#lines);
    this.#done.push(command);
    console.log(`redo: ${command.describe()}`);
  }

  show(): void {
    if (this.#lines.length === 0) {
      console.log("(empty)");
    }
    this.#lines.forEach((line, i) => console.log(`${i + 1}: ${line}`));
  }
}

function run(editor: Editor, words: string[]): void {
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
