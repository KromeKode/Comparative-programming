import { readFileSync } from "node:fs";

// Clockwise order, so turning right adds 1 and turning left subtracts 1.
const DIRECTIONS = ["north", "east", "south", "west"] as const;
type Direction = (typeof DIRECTIONS)[number];

const STEP: Record<Direction, { dx: number; dy: number }> = {
  north: { dx: 0, dy: 1 },
  east: { dx: 1, dy: 0 },
  south: { dx: 0, dy: -1 },
  west: { dx: -1, dy: 0 },
};

type Robot = { x: number; y: number; facing: Direction };

function turn(facing: Direction, steps: number): Direction {
  // Adding the length first keeps the left operand of % from going negative.
  const index = DIRECTIONS.indexOf(facing) + steps + DIRECTIONS.length;
  return DIRECTIONS[index % DIRECTIONS.length];
}

function run(robot: Robot, command: string): void {
  switch (command) {
    case "L":
      robot.facing = turn(robot.facing, -1);
      break;
    case "R":
      robot.facing = turn(robot.facing, 1);
      break;
    case "F":
      robot.x += STEP[robot.facing].dx;
      robot.y += STEP[robot.facing].dy;
      break;
  }
}

const lines = readFileSync(0, "utf8").split("\n");
for (const line of lines) {
  if (line === "") continue;
  const commands = [...line];
  const bad = commands.findIndex((c) => !"LRF".includes(c));
  if (bad !== -1) {
    const command = commands[bad];
    console.log(`error: unknown command '${command}' at position ${bad + 1}`);
    continue;
  }
  const robot: Robot = { x: 0, y: 0, facing: "north" };
  for (const command of commands) {
    run(robot, command);
  }
  console.log(`${robot.x} ${robot.y} ${robot.facing}`);
}
