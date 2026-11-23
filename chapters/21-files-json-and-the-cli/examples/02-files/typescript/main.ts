// Removes repeated lines from each file named on the command line, keeping the first of
// each, and rewrites the file in place. It writes a new file next to the old one and
// renames it over the old one, so that a crash halfway leaves the old file whole.
import {
  closeSync,
  fchmodSync,
  fsyncSync,
  openSync,
  readFileSync,
  renameSync,
  rmSync,
  statSync,
  writeFileSync,
} from "node:fs";

// Rewrites the file without its repeated lines, and returns how many there were.
function dedup(path: string): number {
  // "utf8" decodes the bytes; any that aren't UTF-8 become U+FFFD, without an error.
  const lines = readFileSync(path, "utf8").split("\n");
  if (lines.at(-1) === "") lines.pop(); // the file ended with a newline, or was empty
  const kept = [...new Set(lines)]; // a Set keeps the first of each, in order
  const removed = lines.length - kept.length;
  if (removed === 0) return 0;

  // Node can't make a file with a fresh name, so the new file is FILE.tmp, and "wx"
  // fails if that is already there rather than writing over it.
  const temporary = `${path}.tmp`;
  const fd = openSync(temporary, "wx");
  try {
    try {
      writeFileSync(fd, kept.map((line) => line + "\n").join(""));
      fsyncSync(fd); // onto the disk before the rename
      fchmodSync(fd, statSync(path).mode); // the old file's permissions
    } finally {
      closeSync(fd);
    }
    renameSync(temporary, path);
  } catch (error) {
    rmSync(temporary, { force: true });
    throw error;
  }
  return removed;
}

const paths = process.argv.slice(2);
if (paths.length === 0) {
  console.error("usage: dedup FILE...");
  process.exit(2);
}
for (const path of paths) {
  let removed: number;
  try {
    removed = dedup(path);
  } catch (error) {
    // The message has the error's code, what failed, and the path, such as
    // "ENOENT: no such file or directory, open 'notes.txt'".
    console.error(`dedup: ${error instanceof Error ? error.message : error}`);
    process.exitCode = 1;
    continue;
  }
  console.log(`${path}: removed ${removed} line${removed === 1 ? "" : "s"}`);
}
