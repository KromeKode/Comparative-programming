// Copies stdin to stdout, with each date in it written as YYYY-MM-DD.
import 'dart:convert';
import 'dart:io';

const months = [
  'January', 'February', 'March', 'April', 'May', 'June', //
  'July', 'August', 'September', 'October', 'November', 'December',
];

// Each month's name, or its first three letters: "Jan(?:uary)?", and so on.
final month = months
    .map((n) => n.length > 3 ? '${n.substring(0, 3)}(?:${n.substring(3)})?' : n)
    .join('|');
const d = '([0-9]{1,2})', m = '([0-9]{1,2})', y = '([0-9]{4})';
// In Dart, \b looks at ASCII letters, digits, and _ alone, and \d is only [0-9].
final date = RegExp('\\b(?:$d/$m/$y|($month) $d, $y|$d ($month) $y)\\b');

int daysIn(int month, int year) => switch (month) {
  2 when year % 4 == 0 && (year % 100 != 0 || year % 400 == 0) => 29,
  2 => 28,
  4 || 6 || 9 || 11 => 30,
  _ => 31,
};

int monthOf(String name) => months.indexWhere((m) => m.startsWith(name)) + 1;

/// The date as YYYY-MM-DD, or the text as it was if it isn't a real date.
String rewrite(Match match) {
  // Only one of the three forms matched, so the groups of the other two are null.
  final g = match.groups([1, 2, 3, 4, 5, 6, 7, 8, 9]);
  final (day, month, year) = switch (g) {
    [final d?, final m?, final y?, ...] => (
      int.parse(d),
      int.parse(m),
      int.parse(y),
    ),
    [_, _, _, final n?, final d?, final y?, ...] => (
      int.parse(d),
      monthOf(n),
      int.parse(y),
    ),
    [..., final d?, final n?, final y?] => (
      int.parse(d),
      monthOf(n),
      int.parse(y),
    ),
    _ => throw StateError('no form matched'),
  };
  if (month < 1 || month > 12 || day < 1 || day > daysIn(month, year)) {
    return match[0]!; // not a real date, so left as it was
  }
  String pad(int n, int width) => '$n'.padLeft(width, '0');
  return '${pad(year, 4)}-${pad(month, 2)}-${pad(day, 2)}';
}

Future<void> main() async {
  final text = await utf8.decodeStream(stdin);
  stdout.write(text.replaceAllMapped(date, rewrite));
}
