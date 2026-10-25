import 'dart:io';

void main() {
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    if (line.trim().isEmpty) continue;
    final x = int.parse(line.trim());
    // Dart has no 8-bit type, but int can truncate itself to any width.
    print('$x -> u8 ${x.toUnsigned(8)}, i8 ${x.toSigned(8)}');
  }
}
