// A record (Dart 3) bundles several values without declaring a class.
(int, int) minMax(List<int> values) {
  var lowest = values[0];
  var highest = values[0];
  for (final value in values) {
    if (value < lowest) lowest = value;
    if (value > highest) highest = value;
  }
  return (lowest, highest);
}

// Record fields can have names too.
({int quotient, int remainder}) divMod(int a, int b) =>
    (quotient: a ~/ b, remainder: a.remainder(b));

void main() {
  final (lowest, highest) = minMax([3, -1, 4, 1, 5]);
  print('min and max of [3, -1, 4, 1, 5]: $lowest $highest');

  final (:quotient, :remainder) = divMod(17, 5); // :name unpacks a named field
  print('17 divided by 5: $quotient remainder $remainder');

  var first = 'hello';
  var second = 'world';
  (first, second) = (second, first);
  print('swapped: $first $second');
}
