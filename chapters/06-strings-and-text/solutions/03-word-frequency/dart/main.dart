import 'dart:io';

// A word is a run of ASCII letters and non-ASCII characters.
final wordPattern = RegExp(r'[A-Za-z\u0080-\u{10FFFF}]+', unicode: true);
final asciiUpper = RegExp(r'[A-Z]');

void main() {
  final lines = <String>[];
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    lines.add(line);
  }

  final counts = <String, int>{};
  for (final match in wordPattern.allMatches(lines.join('\n'))) {
    final word = match[0]!.replaceAllMapped(
      asciiUpper,
      (m) => m[0]!.toLowerCase(),
    );
    counts[word] = (counts[word] ?? 0) + 1;
  }

  // Most frequent first; ties in UTF-16 code unit order, which is how compareTo works.
  final sorted = counts.entries.toList()
    ..sort((a, b) {
      final byCount = b.value.compareTo(a.value);
      return byCount != 0 ? byCount : a.key.compareTo(b.key);
    });
  for (final entry in sorted) {
    print('${entry.value} ${entry.key}');
  }
}
