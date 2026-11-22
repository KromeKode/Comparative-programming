import 'dart:io';

// package: names a file in a package's lib folder. "as" puts its names under
// a prefix, so that they read as textstats.wordCount.
import 'package:textstats/textstats.dart' as textstats;

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
