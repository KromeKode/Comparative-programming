import 'dart:io';

const max = 2147483647;
const min = -2147483648;

// A sealed class: the switch in message must cover every subclass.
sealed class ParseError implements Exception {
  const ParseError();

  String get message => switch (this) {
    Empty() => 'empty input',
    NoDigits() => 'no digits after the sign',
    InvalidCharacter(:final character, :final position) =>
      "invalid character '$character' at position $position",
    TooLarge() => 'too large, the maximum is $max',
    TooSmall() => 'too small, the minimum is $min',
  };

  @override
  String toString() => message;
}

class Empty extends ParseError {
  const Empty();
}

class NoDigits extends ParseError {
  const NoDigits();
}

class InvalidCharacter extends ParseError {
  final String character;
  final int position;
  const InvalidCharacter(this.character, this.position);
}

class TooLarge extends ParseError {
  const TooLarge();
}

class TooSmall extends ParseError {
  const TooSmall();
}

bool isDigit(String character) =>
    character.compareTo('0') >= 0 && character.compareTo('9') <= 0;

int parseInt32(String text) {
  if (text.isEmpty) throw const Empty();
  final negative = text[0] == '-';
  final start = text[0] == '+' || text[0] == '-' ? 1 : 0;
  if (start == text.length) throw const NoDigits();
  // Check every character first, so a bad one is reported even when the
  // digits before it are already too large.
  for (var i = start; i < text.length; i++) {
    if (!isDigit(text[i])) throw InvalidCharacter(text[i], i + 1);
  }
  // Stop growing once the number is past every limit, so it cannot
  // overflow however many digits there are.
  var magnitude = 0;
  for (var i = start; i < text.length; i++) {
    final digit = text.codeUnitAt(i) - 48;
    magnitude = (magnitude * 10 + digit).clamp(0, max + 2);
  }
  if (!negative && magnitude > max) throw const TooLarge();
  if (negative && -magnitude < min) throw const TooSmall();
  return negative ? -magnitude : magnitude;
}

void main() {
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    try {
      print('"$line" -> ${parseInt32(line)}');
    } on ParseError catch (error) {
      print('"$line" -> error: $error');
    }
  }
}
