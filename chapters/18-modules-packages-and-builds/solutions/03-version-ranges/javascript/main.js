// Exercise 18.3: version ranges. The highest version that each range allows.
//
// The versions and ranges are semver.js's business; this file reads the input and
// prints the answers.
import { readFileSync } from "node:fs";

import { parseRange, parseVersion } from "./semver.js";

function main() {
  const lines = readFileSync(0, "utf8").split("\n");
  if (lines.at(-1) === "") lines.pop();
  if (lines.length === 0) return;
  const versions = [];
  for (const word of lines[0].split(/\s+/).filter((word) => word !== "")) {
    const version = parseVersion(word);
    if (version === null) {
      console.log(`error: line 1: ${word} isn't a version`);
      return;
    }
    versions.push(version);
  }
  if (versions.length === 0) {
    console.log("error: line 1 must list the versions");
    return;
  }
  for (const [index, raw] of lines.entries()) {
    const line = raw.trim();
    if (index === 0 || line === "") continue;
    let range;
    try {
      range = parseRange(line);
    } catch (error) {
      if (!(error instanceof SyntaxError)) throw error;
      console.log(`error: line ${index + 1}: ${error.message}`);
      continue;
    }
    console.log(`${line} -> ${range.highest(versions) ?? "none"}`);
  }
}

main();
