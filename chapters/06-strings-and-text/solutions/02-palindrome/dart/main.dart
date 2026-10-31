import 'dart:io';

final letterOrDigit = RegExp(r'^[A-Za-z0-9]$');

// runes walks code points. text.split('') would give UTF-16 code units, and
// reversing those tears every emoji in half.
bool isPalindrome(String text) {
  final kept = text.runes
      .map(String.fromCharCode)
      .where((c) => c.codeUnitAt(0) > 0x7F || letterOrDigit.hasMatch(c))
      .map((c) => c.codeUnitAt(0) > 0x7F ? c : c.toLowerCase())
      .toList();
  return kept.join() == kept.reversed.join();
}

void main() {
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    final text = line.trim();
    if (text.isEmpty) continue;
    print('$text: ${isPalindrome(text) ? 'palindrome' : 'not a palindrome'}');
  }
}
