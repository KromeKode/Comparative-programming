import { readFileSync } from "node:fs";

type Grid = string[][];

// [...grid] would copy only the outer array, and the copy would share its
// rows with the grid. Copy each row too.
const clone = (grid: Grid): Grid => grid.map((row) => [...row]);

// Array(height).fill(row) would put one row in every slot, so make each row.
const makeGrid = (width: number, height: number, c: string): Grid =>
  Array.from({ length: height }, () => Array<string>(width).fill(c));

function parseNumber(text: string): number | null {
  if (!/^[0-9]+$/.test(text)) {
    console.log(`error: "${text}" is not a number`);
    return null;
  }
  return Number(text);
}

function oneCharacter(text: string): string | null {
  if ([...text].length !== 1) {
    console.log(`error: "${text}" is not one character`);
    return null;
  }
  return text;
}

function takes(command: string, want: number, got: number): boolean {
  if (got !== want) {
    const plural = want === 1 ? "" : "s";
    console.log(`error: ${command} takes ${want} value${plural}, got ${got}`);
  }
  return got === want;
}

const lines = readFileSync(0, "utf8").split("\n");
if (lines[0] !== "") {
  const [width, height] = lines[0].trim().split(/\s+/).map(Number);
  let grid = makeGrid(width, height, ".");
  let saved: Grid | null = null;

  for (const line of lines.slice(1)) {
    const parts = line.split(/\s+/).filter((p) => p !== "");
    if (parts.length === 0) continue;
    const [command, ...values] = parts;
    switch (command) {
      case "paint": {
        if (!takes(command, 3, values.length)) break;
        const x = parseNumber(values[0]);
        if (x === null) break;
        const y = parseNumber(values[1]);
        if (y === null) break;
        if (x < 1 || x > width || y < 1 || y > height) {
          const where = `(${values[0]}, ${values[1]})`;
          const size = `${width} by ${height}`;
          console.log(`error: ${where} is outside the ${size} grid`);
          break;
        }
        const c = oneCharacter(values[2]);
        if (c !== null) grid[y - 1][x - 1] = c;
        break;
      }
      case "fill": {
        if (!takes(command, 1, values.length)) break;
        const c = oneCharacter(values[0]);
        if (c !== null) grid = makeGrid(width, height, c);
        break;
      }
      case "save":
        if (takes(command, 0, values.length)) saved = clone(grid);
        break;
      case "restore":
        if (!takes(command, 0, values.length)) break;
        if (saved === null) {
          console.log("error: nothing saved");
        } else {
          // grid = saved would share one grid, and the next paint would
          // change the snapshot as well.
          grid = clone(saved);
        }
        break;
      case "show":
        if (takes(command, 0, values.length)) {
          for (const row of grid) console.log(row.join(""));
          console.log("");
        }
        break;
      default:
        console.log(`error: unknown command "${command}"`);
    }
  }
}
