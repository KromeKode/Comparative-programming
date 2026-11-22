import 'package:test/test.dart';
import 'package:wrap/wrap.dart';

void main() {
  const cases = [
    // Lines as long as they can be, and a last line that is shorter.
    ('the quick brown fox jumps', 10, ['the quick', 'brown fox', 'jumps']),
    // A line can be exactly the width, and no longer: "ab cd" is 5.
    ('the quick brown', 9, ['the quick', 'brown']),
    ('ab cd', 4, ['ab', 'cd']),
    ('ab cd', 5, ['ab cd']),
    // One space between words, whatever the text had.
    ('  the   quick  ', 20, ['the quick']),
    // No words, no lines.
    ('', 10, <String>[]),
    ('   ', 10, <String>[]),
    // A word longer than the width gets a line of its own, wherever it is.
    ('extraordinary', 5, ['extraordinary']),
    ('an extraordinary day', 5, ['an', 'extraordinary', 'day']),
    ('a b', 1, ['a', 'b']),
  ];

  for (final (text, width, want) in cases) {
    test('wrap("$text", $width)', () => expect(wrap(text, width), want));
  }
}
