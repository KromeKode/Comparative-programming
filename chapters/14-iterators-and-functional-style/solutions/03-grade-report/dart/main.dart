import 'dart:io';

typedef Grade = ({String name, String subject, int score});

// Core Dart has no groupBy (package:collection has one), so this fills a map
// of lists. ??= makes the list the first time a key turns up.
Map<String, List<Grade>> groupBy(
  Iterable<Grade> grades,
  String Function(Grade) key,
) {
  final groups = <String, List<Grade>>{};
  for (final grade in grades) {
    (groups[key(grade)] ??= []).add(grade);
  }
  return groups;
}

int total(Iterable<Grade> grades) => grades.fold(0, (sum, g) => sum + g.score);

List<String> report(List<Grade> grades) {
  final lines = ['subjects:'];
  final subjects = groupBy(grades, (g) => g.subject);
  for (final subject in subjects.keys.toList()..sort()) {
    final group = subjects[subject]!;
    final best = group.map((g) => g.score).reduce((a, b) => a > b ? a : b);
    final names =
        group.where((g) => g.score == best).map((g) => g.name).toList()..sort();
    final average = total(group) ~/ group.length;
    lines.add('  $subject: average $average, best $best (${names.join(', ')})');
  }

  final students = groupBy(grades, (g) => g.name);
  final ranked =
      students.entries.map((e) => (name: e.key, total: total(e.value))).toList()
        ..sort((a, b) {
          final byTotal = b.total.compareTo(a.total);
          return byTotal != 0 ? byTotal : a.name.compareTo(b.name);
        });
  lines.add('ranking:');
  lines.addAll(
    ranked.indexed.map((e) => '  ${e.$1 + 1}. ${e.$2.name} ${e.$2.total}'),
  );

  final passed =
      students.entries
          .where((e) => e.value.every((g) => g.score >= 50))
          .map((e) => e.key)
          .toList()
        ..sort();
  lines.add(
    'passed everything: ${passed.isEmpty ? 'nobody' : passed.join(', ')}',
  );
  return lines;
}

void main() {
  final grades = <Grade>[];
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    final words = line.trim().split(RegExp(r'\s+'));
    if (words.length < 3) continue;
    grades.add((name: words[0], subject: words[1], score: int.parse(words[2])));
  }
  print(report(grades).join('\n'));
}
