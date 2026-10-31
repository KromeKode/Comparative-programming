// TypeScript's own enum makes code that runs, which --erasableSyntaxOnly rules out.
// A union of string literals, built from one array, is the usual choice instead.
const LIGHTS = ["red", "green", "yellow"] as const;
type Light = (typeof LIGHTS)[number]; // "red" | "green" | "yellow"

const SECONDS: Record<Light, number> = { red: 30, green: 25, yellow: 5 };

const next = (light: Light): Light =>
  LIGHTS[(LIGHTS.indexOf(light) + 1) % LIGHTS.length];

// The compiler knows these three cases cover every Light, so no default is needed.
function meaning(light: Light): string {
  switch (light) {
    case "red":
      return "stop";
    case "green":
      return "go";
    case "yellow":
      return "slow down";
  }
}

const parseLight = (text: string): Light | undefined =>
  LIGHTS.find((light) => light === text);

function showParse(text: string): void {
  console.log(`parse "${text}": ${parseLight(text) ?? "not a light"}`);
}

console.log("lights:", LIGHTS.join(" "));
console.log("yellow is number", LIGHTS.indexOf("yellow"));

let light: Light = "red";
for (const _ of LIGHTS) {
  console.log(`after ${light}: ${next(light)}`);
  light = next(light);
}
console.log(
  `red means ${meaning("red")}, green means ${meaning("green")}, ` +
    `yellow means ${meaning("yellow")}`,
);

const total = LIGHTS.reduce((sum, each) => sum + SECONDS[each], 0);
console.log(`one cycle: ${total} seconds`);

showParse("green");
showParse("blue");
