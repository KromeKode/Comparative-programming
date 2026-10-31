// JavaScript has no enums. A frozen array of strings is a common stand-in.
const LIGHTS = Object.freeze(["red", "green", "yellow"]);
const SECONDS = Object.freeze({ red: 30, green: 25, yellow: 5 });

const next = (light) => LIGHTS[(LIGHTS.indexOf(light) + 1) % LIGHTS.length];

function meaning(light) {
  switch (light) {
    case "red":
      return "stop";
    case "green":
      return "go";
    case "yellow":
      return "slow down";
    default:
      throw new Error(`not a light: ${light}`);
  }
}

// find returns undefined when nothing matches.
const parseLight = (text) => LIGHTS.find((light) => light === text);

function showParse(text) {
  console.log(`parse "${text}": ${parseLight(text) ?? "not a light"}`);
}

console.log("lights:", LIGHTS.join(" "));
console.log("yellow is number", LIGHTS.indexOf("yellow"));

let light = "red";
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
