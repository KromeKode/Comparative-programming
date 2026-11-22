// Write tests for parseDuration and formatDuration here, with test(). One is here to
// start with, and the README lists what they promise.
import 'package:duration/duration.dart';
import 'package:test/test.dart';

void main() {
  test('parse', () => expect(parseDuration('1h30m'), 5400));
}
