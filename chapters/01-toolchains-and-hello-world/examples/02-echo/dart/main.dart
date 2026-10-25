import 'dart:io';

void main() {
  final line = stdin.readLineSync() ?? ''; // null means end of input
  print('You typed: $line');
}
