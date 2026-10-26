import 'dart:io';

String guessesText(int count) => count == 1 ? '1 guess' : '$count guesses';

void main() {
  final secret = int.parse(stdin.readLineSync()!.trim());

  var guesses = 0;
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    final text = line.trim();
    if (text.isEmpty) continue; // a blank line
    final guess = int.tryParse(
      text,
    ); // null when the text is not a whole number
    if (guess == null) {
      print('$text: not a number');
      continue;
    }
    guesses++;
    // Relational patterns (Dart 3) match on the result of compareTo.
    switch (guess.compareTo(secret)) {
      case < 0:
        print('$guess: too low');
      case > 0:
        print('$guess: too high');
      default:
        print('$guess: correct in ${guessesText(guesses)}');
        return;
    }
  }
  print('gave up after ${guessesText(guesses)}; the number was $secret');
}
