const grid = [
  [3, 8, 1],
  [9, 4, 7],
  [2, 6, 5],
];

void main() {
  targets:
  for (final target in [7, 10]) {
    var checked = 0;
    for (final (r, row) in grid.indexed) {
      for (final (c, value) in row.indexed) {
        checked++;
        if (value == target) {
          print('$target: row $r, column $c (checked $checked cells)');
          continue targets; // leaves both inner loops at once
        }
      }
    }
    print('$target: not found (checked $checked cells)');
  }
}
