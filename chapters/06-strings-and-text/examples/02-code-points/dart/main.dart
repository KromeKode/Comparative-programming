import 'dart:convert';

String hex(int n, int width) =>
    n.toRadixString(16).toUpperCase().padLeft(width, '0');

void main() {
  const s = 'Zo\u00eb \u{1F44D}\u{1F3FD}';

  final bytes = utf8.encode(s).map((b) => hex(b, 2));
  print('UTF-8 bytes: ${bytes.join(' ')}');

  // s.runes walks code points; s[i] and s.length count UTF-16 code units.
  final codePoints = s.runes.map((r) => 'U+${hex(r, 4)}');
  print('code points: ${codePoints.join(' ')}');

  print('first 3 code points: ${String.fromCharCodes(s.runes.take(3))}');
}
