import 'dart:io';

int countOnes(int n) {
  var count = 0;
  for (var rest = n; rest != 0; rest &= rest - 1) {
    count++; // rest & (rest - 1) clears the lowest set bit
  }
  return count;
}

void main() {
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    final text = line.trim();
    if (text.isEmpty) continue;
    final n = int.parse(text);
    print('$n: ${n.toRadixString(2)} (ones ${countOnes(n)}, width ${n.bitLength})');
  }
}
