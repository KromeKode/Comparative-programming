// (int, int) is a record type (Dart 3). Switch patterns can take it apart.
String classify((int, int) point) => switch (point) {
  (0, 0) => 'origin',
  (var x, 0) => 'on the x axis at x = $x', // x binds the first value
  (0, var y) => 'on the y axis at y = $y',
  (var x, var y) when x == y => 'on the diagonal at $x', // a guard
  (var x, var y) => 'x = $x, y = $y',
};

void main() {
  for (final (x, y) in [(0, 0), (3, 0), (0, -2), (4, 4), (-1, 5)]) {
    print('($x, $y): ${classify((x, y))}');
  }
}
