import { readFileSync } from "node:fs";

// A Map keeps its keys in the order they were added, which is the order the groups
// must be printed in.
const groups = new Map();
for (const raw of readFileSync(0, "utf8").split(/\s+/)) {
  if (raw === "") continue;
  const word = raw.toLowerCase();
  const key = [...word].sort().join(""); // anagrams have the same letters once sorted
  if (!groups.has(key)) groups.set(key, []);
  groups.get(key).push(word);
}

for (const group of groups.values()) {
  console.log(group.join(" "));
}
