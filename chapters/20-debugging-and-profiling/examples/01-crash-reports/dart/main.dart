// Counts the letters in its input, and draws a bar for each one. It expects lowercase
// text, and doesn't check: section 20.3 gives it capitals, to see what happens.
import 'dart:io';

final letter = RegExp(r'\p{L}', unicode: true);

List<int> countLetters(String text) {
  final counts = List.filled(26, 0);
  for (final c in text.split('')) {
    if (letter.hasMatch(c)) counts[c.codeUnitAt(0) - 'a'.codeUnitAt(0)]++;
  }
  return counts;
}

void main() {
  final lines = <String>[];
  for (String? line; (line = stdin.readLineSync()) != null;) {
    lines.add(line!);
  }
  final counts = countLetters(lines.join('\n'));
  for (var i = 0; i < 26; i++) {
    if (counts[i] > 0) {
      print('${String.fromCharCode(0x61 + i)} ${counts[i]} ${'#' * counts[i]}');
    }
  }
}
