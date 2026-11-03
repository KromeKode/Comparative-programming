import 'dart:io';

class LineError implements Exception {
  final String message;
  LineError(this.message);

  @override
  String toString() => message;
}

class Version implements Comparable<Version> {
  final int major, minor, patch;
  Version(this.major, this.minor, this.patch);

  // Part by part, as numbers: major first, then minor, then patch.
  @override
  int compareTo(Version other) {
    if (major != other.major) return major.compareTo(other.major);
    if (minor != other.minor) return minor.compareTo(other.minor);
    return patch.compareTo(other.patch);
  }

  @override
  String toString() => '$major.$minor.$patch';
}

// int is Comparable<num>, not Comparable<int>, so the bound is
// Comparable<Object>, which int, String, and Version all meet.
class Interval<T extends Comparable<Object>> {
  final T low;
  T high;
  Interval(this.low, this.high);

  @override
  String toString() => '$low..$high';
}

// Sort by the low ends, then join each interval to the last one if they overlap.
List<Interval<T>> merge<T extends Comparable<Object>>(
  List<Interval<T>> intervals,
) {
  final sorted = [...intervals]..sort((a, b) => a.low.compareTo(b.low));
  final merged = <Interval<T>>[];
  for (final interval in sorted) {
    if (merged.isNotEmpty && interval.low.compareTo(merged.last.high) <= 0) {
      if (interval.high.compareTo(merged.last.high) > 0)
        merged.last.high = interval.high;
    } else {
      merged.add(Interval(interval.low, interval.high));
    }
  }
  return merged;
}

final whole = RegExp(r'^-?[0-9]+$');
final digits = RegExp(r'^[0-9]+$');
final letters = RegExp(r'^[a-z]+$');

int? parseInt(String text) => whole.hasMatch(text) ? int.tryParse(text) : null;

String? parseWord(String text) => letters.hasMatch(text) ? text : null;

Version? parseVersion(String text) {
  final parts = text.split('.');
  if (parts.length != 3 || !parts.every(digits.hasMatch)) return null;
  final [major, minor, patch] = parts.map(int.parse).toList();
  return Version(major, minor, patch);
}

// Parse every interval on a line, merge them, and format the result.
String run<T extends Comparable<Object>>(
  String kind,
  T? Function(String) parse,
  List<String> words,
) {
  final intervals = [
    for (final word in words) parseInterval(kind, parse, word),
  ];
  return [kind, ...merge(intervals)].join(' ');
}

Interval<T> parseInterval<T extends Comparable<Object>>(
  String kind,
  T? Function(String) parse,
  String word,
) {
  final ends = word.split('..');
  if (ends.length != 2) throw LineError('bad interval "$word"');
  final low = parse(ends[0]) ?? (throw LineError('bad $kind "${ends[0]}"'));
  final high = parse(ends[1]) ?? (throw LineError('bad $kind "${ends[1]}"'));
  if (high.compareTo(low) < 0) throw LineError('$word is backwards');
  return Interval(low, high);
}

void main() {
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    final words = line.trim().split(RegExp(r'\s+'));
    if (words.first.isEmpty) continue;
    final [kind, ...rest] = words;
    try {
      final text = switch (kind) {
        'int' => run(kind, parseInt, rest),
        'word' => run(kind, parseWord, rest),
        'version' => run(kind, parseVersion, rest),
        _ => throw LineError('unknown kind "$kind"'),
      };
      print(text);
    } on LineError catch (error) {
      print('error: $error');
    }
  }
}
