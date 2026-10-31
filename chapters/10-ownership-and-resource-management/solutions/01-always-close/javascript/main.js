import { readFileSync } from "node:fs";

const MAX_OPEN = 2;
const MAX_LINES = 8;

class DiskError extends Error {}

class Disk {
  files = new Map(); // name -> { lines, locked }
  openCount = 0;

  linesUsed() {
    let total = 0;
    for (const file of this.files.values()) total += file.lines.length;
    return total;
  }

  // Returns an open Handle. The caller must close it, in a finally block.
  open(name, writing = false) {
    if (this.openCount === MAX_OPEN) {
      throw new DiskError("too many open files");
    }
    const file = this.files.get(name);
    if (writing) {
      if (file?.locked) throw new DiskError(`"${name}" is read-only`);
      this.files.set(name, { lines: [], locked: false });
    } else if (file === undefined) {
      throw new DiskError(`no file "${name}"`);
    }
    this.openCount++;
    console.log(`open ${name}`);
    return new Handle(this, name);
  }
}

class Handle {
  constructor(disk, name) {
    this.disk = disk;
    this.name = name;
  }

  lines() {
    return [...this.disk.files.get(this.name).lines];
  }

  write(line) {
    if (this.disk.linesUsed() === MAX_LINES) {
      throw new DiskError("disk full");
    }
    this.disk.files.get(this.name).lines.push(line);
  }

  close() {
    this.disk.openCount--;
    console.log(`close ${this.name}`);
  }
}

const lines = (count) => (count === 1 ? "1 line" : `${count} lines`);

function writeFile(disk, name, words) {
  const target = disk.open(name, true);
  try {
    for (const word of words) target.write(word);
  } finally {
    target.close();
  }
  console.log(`wrote ${lines(words.length)}`);
}

function catFile(disk, name) {
  const source = disk.open(name);
  try {
    for (const line of source.lines()) console.log(`  ${line}`);
  } finally {
    source.close();
  }
}

function copyFile(disk, from, to) {
  if (from === to) throw new DiskError(`cannot copy "${from}" onto itself`);
  let copied = 0;
  const source = disk.open(from);
  try {
    const target = disk.open(to, true);
    try {
      for (const line of source.lines()) {
        target.write(line);
        copied++;
      }
    } finally {
      target.close();
    }
  } finally {
    source.close();
  }
  console.log(`copied ${lines(copied)}`);
}

function lockFile(disk, name) {
  const file = disk.files.get(name);
  if (file === undefined) throw new DiskError(`no file "${name}"`);
  file.locked = true;
  console.log(`locked ${name}`);
}

function takes(command, want, args) {
  if (args.length !== want) {
    const noun = want === 1 ? "name" : "names";
    throw new DiskError(`${command} takes ${want} ${noun}, got ${args.length}`);
  }
}

function run(disk, command, args) {
  switch (command) {
    case "write":
      if (args.length === 0) throw new DiskError("write needs a file name");
      return writeFile(disk, args[0], args.slice(1));
    case "cat":
      takes(command, 1, args);
      return catFile(disk, args[0]);
    case "copy":
      takes(command, 2, args);
      return copyFile(disk, args[0], args[1]);
    case "lock":
      takes(command, 1, args);
      return lockFile(disk, args[0]);
    default:
      throw new DiskError(`unknown command "${command}"`);
  }
}

const disk = new Disk();
for (const line of readFileSync(0, "utf8").split("\n")) {
  const parts = line.split(/\s+/).filter((part) => part !== "");
  if (parts.length === 0) continue;
  try {
    run(disk, parts[0], parts.slice(1));
  } catch (error) {
    if (!(error instanceof DiskError)) throw error;
    console.log(`error: ${error.message}`);
  }
}
console.log(`files still open: ${disk.openCount}`);
