// Prints a report of each student's scores. ../README.md says what it should print.
import 'dart:io';

/// Gives total / count, rounded to the nearest whole number, with halves up.
int roundedAverage(int total, int count) => (2 * total + count) ~/ (2 * count);

void main(List<String> args) {
  // Each student's scores, in the order the students first appear.
  final scores = <String, List<int>>{};
  // The input is the file named on the command line, if there is one, so that a
  // debugger can keep the terminal to itself.
  final lines = args.isNotEmpty
      ? File(args[0]).readAsLinesSync()
      : [for (String? line; (line = stdin.readLineSync()) != null;) line!];
  for (final line in lines) {
    final fields = line.trim().split(RegExp(r'\s+'));
    if (fields.first.isEmpty) continue;
    final name = fields.first;
    final marks = [for (final field in fields.skip(1)) int.parse(field)];
    scores.putIfAbsent(name, () => []).addAll(marks);
  }

  var bestName = '-';
  var bestAverage = -1;
  var classTotal = 0;
  var classCount = 0;
  for (final MapEntry(key: name, value: marks) in scores.entries) {
    var total = 0;
    for (final mark in marks) {
      total += mark;
    }
    classTotal += total;
    classCount += marks.length;
    if (marks.isEmpty) {
      print('$name -');
      continue;
    }
    final average = roundedAverage(total, marks.length);
    print('$name $average');
    if (average > bestAverage) {
      bestName = name;
      bestAverage = average;
    }
  }

  if (classCount == 0) {
    print('class average: -');
  } else {
    print('class average: ${roundedAverage(classTotal, classCount)}');
  }
  print('top: $bestName');
}
