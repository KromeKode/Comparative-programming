import 'dart:io';

void main() {
  final name = (stdin.readLineSync() ?? '').trim();
  if (name.isEmpty) {
    print('Hello, stranger!');
  } else {
    print('Hello, $name!');
  }
}
