import 'package:roman/roman.dart';
import 'package:test/test.dart';

void main() {
  const cases = [
    (1, 'I'),
    (4, 'IV'),
    (9, 'IX'),
    (14, 'XIV'),
    (40, 'XL'),
    (90, 'XC'),
    (400, 'CD'),
    (1994, 'MCMXCIV'),
    (2024, 'MMXXIV'),
    (3999, 'MMMCMXCIX'),
  ];

  // A group names the tests in it, and a test for each case, named after it,
  // says which case failed.
  group('toRoman', () {
    for (final (n, want) in cases) {
      test('$n', () => expect(toRoman(n), want));
    }
  });

  // The errors are part of what toRoman promises, so they get tests too.
  test('out of range', () {
    for (final n in [0, 4000, -1]) {
      expect(() => toRoman(n), throwsRangeError);
    }
  });
}
