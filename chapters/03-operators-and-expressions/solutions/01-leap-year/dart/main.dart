import 'dart:io';

bool isLeap(int year) =>
    year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);

void main() {
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    final text = line.trim();
    if (text.isEmpty) continue;
    final year = int.parse(text);
    print('$year: ${isLeap(year) ? 'leap year' : 'common year'}');
  }
}
