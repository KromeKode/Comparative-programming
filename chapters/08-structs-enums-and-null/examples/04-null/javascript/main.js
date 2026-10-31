// alan has no nickname property at all, so reading it gives undefined.
const users = [
  { name: "ada", nickname: "Countess" },
  { name: "alan" },
  { name: "grace", nickname: "Amazing Grace" },
];

// Array find gives undefined when nothing matches.
const find = (name) => users.find((user) => user.name === name);

// a ?? b is b only when a is null or undefined.
for (const user of users) {
  console.log(`${user.name} is shown as ${user.nickname ?? user.name}`);
}

// a?.b stops and gives undefined when a is null or undefined.
const lengths = users.map((user) => user.nickname?.length ?? 0);
console.log("nickname lengths:", lengths.join(" "));

for (const name of ["alan", "linus"]) {
  console.log(`find "${name}": ${find(name)?.name ?? "nobody"}`);
}

for (const name of ["ada", "alan", "linus"]) {
  console.log(`nickname of "${name}": ${find(name)?.nickname ?? "none"}`);
}
