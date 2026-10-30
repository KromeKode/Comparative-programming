import 'dart:io';

final lowerA = 'a'.codeUnitAt(0), upperA = 'A'.codeUnitAt(0);

// Shifts an ASCII letter and leaves every other code unit alone. Working on UTF-16
// code units is safe here: neither half of an emoji is in the ASCII range.
int shiftLetter(int unit, int shift) {
  for (final base in [lowerA, upperA]) {
    if (unit >= base && unit < base + 26) {
      return (unit - base + shift) % 26 + base;
    }
  }
  return unit;
}

void main() {
  while (true) {
    final raw = stdin.readLineSync();
    if (raw == null) break;
    final line = raw.trim();
    if (line.isEmpty) continue;
    final space = line.indexOf(' ');
    final number = space == -1 ? line : line.substring(0, space);
    final text = space == -1 ? '' : line.substring(space + 1);
    final shift = int.parse(number) % 26; // Dart's % is never negative here
    final units = text.codeUnits.map((unit) => shiftLetter(unit, shift));
    print(String.fromCharCodes(units));
  }
}
