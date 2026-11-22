import 'dart:io';

// A relative import names the file. "as" puts its names under a prefix, so
// they read as textstats.wordCount and can't clash with names of this file.
import 'textstats.dart' as textstats;

void main() {
  final lines = <String>[];
  for (String? line; (line = stdin.readLineSync()) != null;) {
    lines.add(line!);
  }
  final text = lines.join('\n');
  print('textstats ${textstats.version}');
  print('words: ${textstats.wordCount(text)}');
  print('longest: ${textstats.longestWord(text)}');
}
