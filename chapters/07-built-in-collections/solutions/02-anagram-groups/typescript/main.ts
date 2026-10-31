import { readFileSync } from "node:fs";

const groups = new Map<string, string[]>();
for (const raw of readFileSync(0, "utf8").split(/\s+/)) {
  if (raw === "") continue;
  const word = raw.toLowerCase();
  const key = [...word].sort().join("");
  let group = groups.get(key); // string[] | undefined
  if (group === undefined) {
    group = [];
    groups.set(key, group);
  }
  group.push(word);
}

for (const group of groups.values()) {
  console.log(group.join(" "));
}
