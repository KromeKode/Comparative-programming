const _numerals = [
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

/// [n] in Roman numerals. Throws a [RangeError] if [n] is outside 1 to 3999.
String toRoman(int n) {
  RangeError.checkValueInInterval(n, 1, 3999, 'n');
  final numeral = StringBuffer();
  for (final (value, letters) in _numerals) {
    for (; n >= value; n -= value) {
      numeral.write(letters);
    }
  }
  return numeral.toString();
}
