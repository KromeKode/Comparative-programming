void main() {
  var line = 'up:';
  for (var i = 0; i < 5; i++) {
    line += ' $i';
  }
  print(line);

  line = 'down:';
  for (var i = 5; i >= 1; i--) {
    line += ' $i';
  }
  print(line);

  line = 'by threes:';
  for (var i = 0; i < 10; i += 3) {
    line += ' $i';
  }
  print(line);

  line = 'halving:';
  var n = 100;
  while (n > 0) {
    line += ' $n';
    n ~/= 2;
  }
  print(line);

  const colors = ['red', 'green', 'blue'];
  line = 'colors:';
  for (final color in colors) {
    line += ' $color';
  }
  print(line);

  line = 'numbered:';
  for (final (i, color) in colors.indexed) {
    line += ' ${i + 1}=$color'; // indexed gives (index, value) records
  }
  print(line);

  // do-while tests the condition after the body, so the body always runs once.
  var m = 100;
  do {
    m *= 2;
  } while (m < 50);
  print('do-while: $m');
}
