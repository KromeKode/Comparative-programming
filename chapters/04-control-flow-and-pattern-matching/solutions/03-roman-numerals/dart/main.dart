import 'dart:io';

const table = [
  (1000, 'M'),
  (900, 'CM'),
  (500, 'D'),
  (400, 'CD'),
  (100, 'C'),
  (90, 'XC'),
  (50, 'L'),
  (40, 'XL'),
  (10, 'X'),
  (9, 'IX'),
  (5, 'V'),
  (4, 'IV'),
  (1, 'I'),
];

String toRoman(int n) {
  final out = StringBuffer();
  for (final (value, symbol) in table) {
    while (n >= value) {
      out.write(symbol); // take the biggest value that still fits
      n -= value;
    }
  }
  return out.toString();
}

int letterValue(String letter) => switch (letter) {
  'I' => 1,
  'V' => 5,
  'X' => 10,
  'L' => 50,
  'C' => 100,
  'D' => 500,
  'M' => 1000,
  _ => throw FormatException('not a Roman numeral letter: $letter'),
};

int fromRoman(String text) {
  var total = 0;
  for (var i = 0; i < text.length; i++) {
    final value = letterValue(text[i]);
    // A smaller letter before a bigger one is subtracted: IV is 5 - 1.
    if (i + 1 < text.length && value < letterValue(text[i + 1])) {
      total -= value;
    } else {
      total += value;
    }
  }
  return total;
}

void main() {
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    final text = line.trim();
    if (text.isEmpty) continue;
    final n = int.tryParse(text);
    if (n != null) {
      print('$n -> ${toRoman(n)}');
    } else {
      print('$text -> ${fromRoman(text)}');
    }
  }
}
