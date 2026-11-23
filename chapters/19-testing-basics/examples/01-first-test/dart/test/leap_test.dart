// dart test runs every file in test/ whose name ends in _test.dart. Its main
// registers the tests by calling test(), and the runner runs them afterward.
import 'package:leap/leap.dart';
import 'package:test/test.dart';

void main() {
  test('divisible by 4', () {
    expect(isLeapYear(2024), isTrue);
    expect(isLeapYear(2023), isFalse);
  });

  test('centuries', () {
    expect(isLeapYear(1900), isFalse);
    expect(isLeapYear(2000), isTrue);
  });
}
