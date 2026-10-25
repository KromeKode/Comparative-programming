import 'dart:io';

void main() {
  final n = int.tryParse((stdin.readLineSync() ?? '').trim());
  if (n == null) {
    stderr.writeln('expected a whole number');
    exit(1); // exit() never returns, so below this line Dart knows n is an int
  }
  for (var i = 1; i <= n; i++) {
    if (i % 15 == 0) {
      print('FizzBuzz');
    } else if (i % 3 == 0) {
      print('Fizz');
    } else if (i % 5 == 0) {
      print('Buzz');
    } else {
      print(i);
    }
  }
}
