import 'dart:io';

void main() {
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    final parts = line.trim().split(RegExp(r'\s+'));
    if (parts.length != 2) continue; // a blank line
    final a = int.parse(parts[0]);
    final b = int.parse(parts[1]);
    final sum = a + b; // native Dart wraps on overflow, like Go
    if ((a > 0 && b > 0 && sum < 0) || (a < 0 && b < 0 && sum >= 0)) {
      print('overflow');
    } else {
      print(sum);
    }
  }
}
