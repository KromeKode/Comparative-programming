import { readFileSync } from "node:fs";

// Each shape is an object whose kind field says which variant it is.
function area(shape) {
  switch (shape.kind) {
    case "circle":
      return Math.PI * shape.radius * shape.radius;
    case "rectangle":
      return shape.width * shape.height;
    case "triangle":
      return (shape.base * shape.height) / 2;
  }
}

const SIZES_NEEDED = new Map([
  ["circle", 1],
  ["rectangle", 2],
  ["triangle", 2],
]);

function parseSize(text) {
  // Number() rejects "3cm"; parseFloat("3cm") would quietly give 3.
  const value = Number(text);
  if (Number.isNaN(value)) throw new Error(`"${text}" is not a number`);
  if (value < 0) throw new Error(`"${text}" is negative`);
  return value;
}

// Turns a shape name and its sizes into a shape, or throws an Error.
function parse(name, texts) {
  const needed = SIZES_NEEDED.get(name);
  if (needed === undefined) throw new Error(`unknown shape "${name}"`);
  if (texts.length !== needed) {
    const plural = needed === 1 ? "" : "s";
    throw new Error(
      `${name} needs ${needed} size${plural}, got ${texts.length}`,
    );
  }
  const [a, b] = texts.map(parseSize);
  switch (name) {
    case "circle":
      return { kind: "circle", radius: a };
    case "rectangle":
      return { kind: "rectangle", width: a, height: b };
    default:
      return { kind: "triangle", base: a, height: b };
  }
}

let total = 0;
for (const line of readFileSync(0, "utf8").split("\n")) {
  const [name, ...texts] = line.split(/\s+/).filter((word) => word !== "");
  if (name === undefined) continue;
  try {
    const shape = parse(name, texts);
    total += area(shape);
    console.log(`${name}: ${area(shape).toFixed(2)}`);
  } catch (error) {
    console.log(`error: ${error.message}`);
  }
}
console.log(`total: ${total.toFixed(2)}`);
