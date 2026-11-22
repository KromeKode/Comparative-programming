import 'dart:io';
import 'dart:isolate';

String plural(int count, String noun) => '$count $noun${count == 1 ? '' : 's'}';

final word = RegExp('[A-Za-z]+');

// Runs in an isolate of its own. The lines arrive as a copy, and the map
// goes back as a copy, so the isolates share nothing.
Map<String, int> countWords(List<String> lines) {
  final counts = <String, int>{};
  for (final line in lines) {
    for (final match in word.allMatches(line)) {
      counts.update(
        match[0]!.toLowerCase(),
        (count) => count + 1,
        ifAbsent: () => 1,
      );
    }
  }
  return counts;
}

int sum(Iterable<int> values) => values.fold(0, (a, b) => a + b);

Future<void> main() async {
  final lines = <String>[];
  for (String? line; (line = stdin.readLineSync()) != null;) {
    lines.add(line!);
  }
  final first = lines.isEmpty
      ? <String>[]
      : lines[0].trim().split(RegExp(r'\s+'));
  final number = RegExp(r'^[0-9]+$');
  if (first.length != 2 || !first.every(number.hasMatch)) {
    print('error: the first line must be two numbers, workers and top');
    return;
  }
  final [workers, top] = first.map(int.parse).toList();
  if (workers < 1 || workers > 16) {
    print('error: workers must be from 1 to 16');
    return;
  }

  // Each worker gets text.length ~/ workers lines, and the first few get one
  // more, so that every line is counted once.
  final text = lines.sublist(1);
  final chunks = <List<String>>[];
  var start = 0;
  for (var i = 0; i < workers; i++) {
    final size = text.length ~/ workers + (i < text.length % workers ? 1 : 0);
    chunks.add(text.sublist(start, start + size));
    start += size;
  }

  // Isolate.run copies the function and what it captures into a new
  // isolate, and returns its result as a Future.
  final counts = await Future.wait([
    for (final chunk in chunks) Isolate.run(() => countWords(chunk)),
  ]);

  print('${plural(workers, 'worker')}, ${plural(text.length, 'line')}');
  final total = <String, int>{};
  for (final (i, chunk) in chunks.indexed) {
    final words = sum(counts[i].values);
    print(
      'worker $i: ${plural(chunk.length, 'line')}, ${plural(words, 'word')}',
    );
    counts[i].forEach((word, count) {
      total.update(word, (old) => old + count, ifAbsent: () => count);
    });
  }
  print('${plural(sum(total.values), 'word')}, ${total.length} different');

  final sorted = total.entries.toList()
    ..sort(
      (a, b) => b.value != a.value ? b.value - a.value : a.key.compareTo(b.key),
    );
  for (final entry in sorted.take(top)) {
    print('${entry.key}: ${entry.value}');
  }
}
