// Write tests for wrap here, with test(). One is here to start with, and the
// README lists what wrap promises.
import 'package:test/test.dart';
import 'package:wrap/wrap.dart';

void main() {
  test('one line', () => expect(wrap('one two', 10), ['one two']));
}
