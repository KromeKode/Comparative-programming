import 'dart:collection';
import 'dart:io';

// An extension adds the adapters to every Iterable<int>, so they chain like
// the built-in ones. Each is a sync* generator that pulls one item at a time.
extension Groups on Iterable<int> {
  Iterable<List<int>> windows(int k) sync* {
    final window = Queue<int>();
    for (final item in this) {
      window.add(item);
      if (window.length > k) window.removeFirst(); // the oldest item leaves
      if (window.length == k)
        yield window.toList(); // a copy, since window changes
    }
  }

  Iterable<List<int>> chunks(int k) sync* {
    var chunk = <int>[];
    for (final item in this) {
      chunk.add(item);
      if (chunk.length == k) {
        yield chunk;
        chunk = [];
      }
    }
    if (chunk.isNotEmpty) yield chunk;
  }

  Iterable<List<int>> runs() sync* {
    var run = <int>[];
    for (final item in this) {
      if (run.isNotEmpty && item != run.first) {
        yield run;
        run = [];
      }
      run.add(item);
    }
    if (run.isNotEmpty) yield run;
  }
}

String show(Iterable<List<int>> groups) =>
    groups.map((group) => '[${group.join(' ')}]').join(' ');

String answer(List<String> words) {
  String text;
  if (words[0] == 'runs') {
    text = show(words.skip(1).map(int.parse).runs());
  } else {
    final k = int.parse(words[1]);
    final numbers = words
        .skip(2)
        .map(int.parse); // lazy: parsed as they are pulled
    text = switch (words[0]) {
      'windows' => show(numbers.windows(k)),
      'chunks' => show(numbers.chunks(k)),
      _ =>
        numbers
            .windows(k)
            .map((window) => window.fold(0, (a, b) => a + b))
            .join(' '),
    };
  }
  return text.isEmpty ? '(none)' : text;
}

void main() {
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    final words = line.trim().split(RegExp(r'\s+'));
    if (words.first.isEmpty) continue;
    print(answer(words));
  }
}
