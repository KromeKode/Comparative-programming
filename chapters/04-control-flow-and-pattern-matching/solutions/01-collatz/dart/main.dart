import 'dart:io';
import 'dart:math';

void main() {
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    final text = line.trim();
    if (text.isEmpty) continue;
    final start = int.parse(text);
    var n = start;
    var peak = n;
    var steps = 0;
    while (n != 1) {
      n = n.isEven ? n ~/ 2 : 3 * n + 1;
      peak = max(peak, n);
      steps++;
    }
    print('$start: steps $steps, peak $peak');
  }
}
