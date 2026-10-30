import 'dart:convert';

void main() {
  // A Dart String is a sequence of UTF-16 code units.
  const s = 'Zo\u00eb \u{1F44D}\u{1F3FD}';

  print('text: $s');
  print('built-in length: ${s.length} (UTF-16 code units)');
  print('UTF-8 bytes: ${utf8.encode(s).length}');
  print('UTF-16 code units: ${s.codeUnits.length}');
  print('code points: ${s.runes.length}'); // runes are code points
}
