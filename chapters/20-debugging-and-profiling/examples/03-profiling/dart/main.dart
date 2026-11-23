// Counts the words in its input, and prints the ten most common. It is slow on a long
// text, and section 20.7 runs it under a profiler to find out why.
import 'dart:io';

typedef Count = ({String word, int n});

final wordPattern = RegExp('[a-z]+');

/// Gives the words of the text: runs of the letters a to z, in lowercase.
List<String> splitWords(String text) => [
  for (final match in wordPattern.allMatches(text.toLowerCase())) match[0]!,
];

/// Gives the index of the word's count, or -1 if it has none.
int find(List<Count> counts, String word) {
  for (var i = 0; i < counts.length; i++) {
    if (counts[i].word == word) return i;
  }
  return -1;
}

List<Count> countWords(List<String> words) {
  final counts = <Count>[];
  for (final word in words) {
    final i = find(counts, word);
    if (i == -1) {
      counts.add((word: word, n: 1));
    } else {
      counts[i] = (word: word, n: counts[i].n + 1);
    }
  }
  return counts;
}

void main() {
  final lines = <String>[];
  for (String? line; (line = stdin.readLineSync()) != null;) {
    lines.add(line!);
  }
  final counts = countWords(splitWords(lines.join('\n')));
  counts.sort((a, b) => a.n != b.n ? b.n - a.n : a.word.compareTo(b.word));
  for (final count in counts.take(10)) {
    print('${count.n} ${count.word}');
  }
}
