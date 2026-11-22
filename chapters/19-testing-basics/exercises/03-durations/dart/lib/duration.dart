/// Durations such as 1h30m: [parseDuration] reads one, and [formatDuration] writes one.
library;

const _units = 'dhms'; // largest first, which is the order they come in
const _seconds = [86400, 3600, 60, 1]; // in each of the units
const _maxInt = 0x7FFFFFFFFFFFFFFF; // the largest int, on the Dart VM

/// The number of seconds that [text] says, such as 5400 for "1h30m".
///
/// Throws a [FormatException], as `int.parse` does, if [text] isn't a duration.
int parseDuration(String text) {
  if (text.isEmpty) throw FormatException("a duration can't be empty", text);
  var seconds = 0;
  var nextUnit = 0; // each part's unit comes after the one before it in _units
  var i = 0;
  while (i < text.length) {
    final start = i;
    while (i < text.length && '0123456789'.contains(text[i])) {
      i++;
    }
    if (i == start) throw FormatException('a part has no number', text, i);
    if (i == text.length) {
      throw FormatException('a number has no unit', text, i);
    }
    final number = int.parse(text.substring(start, i));
    final unit = _units.indexOf(text[i]);
    if (unit == -1) throw FormatException('not a unit', text, i);
    if (unit < nextUnit) {
      throw FormatException('a unit out of order, or twice', text, i);
    }
    if (number > (_maxInt - seconds) ~/ _seconds[unit]) {
      throw FormatException('too long to count', text);
    }
    seconds += number * _seconds[unit];
    nextUnit = unit + 1;
    i++;
  }
  return seconds;
}

/// [seconds] as a duration, such as "1h30m" for 5400, leaving out the units that are 0.
String formatDuration(int seconds) {
  if (seconds == 0) return '0s';
  final text = StringBuffer();
  for (var unit = 0; unit < _units.length; unit++) {
    final amount = seconds ~/ _seconds[unit];
    seconds %= _seconds[unit];
    if (amount > 0) text.write('$amount${_units[unit]}');
  }
  return text.toString();
}
