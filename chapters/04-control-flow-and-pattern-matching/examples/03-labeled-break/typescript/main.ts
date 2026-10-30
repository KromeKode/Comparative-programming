const grid: number[][] = [
  [3, 8, 1],
  [9, 4, 7],
  [2, 6, 5],
];

targets: for (const target of [7, 10]) {
  let checked = 0;
  for (const [r, row] of grid.entries()) {
    for (const [c, value] of row.entries()) {
      checked++;
      if (value === target) {
        console.log(`${target}: row ${r}, column ${c} (checked ${checked} cells)`);
        continue targets; // leaves both inner loops at once
      }
    }
  }
  console.log(`${target}: not found (checked ${checked} cells)`);
}
export {};
