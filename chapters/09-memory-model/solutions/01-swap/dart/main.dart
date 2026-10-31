import 'dart:io';

// A function cannot reach the caller's variables, only the objects they refer
// to, so swap takes the list and two indexes.
void swap(List<String> words, int i, int j) {
  final tmp = words[i];
  words[i] = words[j];
  words[j] = tmp;
}

// Reverses words[start..end] and returns how many swaps that took.
int reverse(List<String> words, int start, int end) {
  var swaps = 0;
  for (; start < end; start++, end--) {
    swap(words, start, end);
    swaps++;
  }
  return swaps;
}

final digits = RegExp(r'^[0-9]+$');

int? parsePosition(String text, int count) {
  if (!digits.hasMatch(text)) {
    print('error: "$text" is not a position');
    return null;
  }
  // tryParse gives null for a number too big for an int: out of range too.
  final value = int.tryParse(text);
  if (value == null || value < 1 || value > count) {
    print('error: position $text is out of range (1 to $count)');
    return null;
  }
  return value;
}

List<String> fields(String line) =>
    line.split(RegExp(r'\s+')).where((part) => part.isNotEmpty).toList();

void main() {
  final first = stdin.readLineSync();
  final words = first == null ? <String>[] : fields(first);
  var swaps = 0;
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    final parts = fields(line);
    if (parts.isEmpty) continue;
    final command = parts[0];
    if (command != 'swap' && command != 'reverse') {
      print('error: unknown command "$command"');
      continue;
    }
    if (parts.length != 3) {
      print('error: $command needs 2 positions, got ${parts.length - 1}');
      continue;
    }
    final i = parsePosition(parts[1], words.length);
    if (i == null) continue;
    final j = parsePosition(parts[2], words.length);
    if (j == null) continue;
    if (command == 'swap') {
      swap(words, i - 1, j - 1);
      swaps++;
    } else {
      final (low, high) = i <= j ? (i, j) : (j, i);
      swaps += reverse(words, low - 1, high - 1);
    }
    print(words.join(' '));
  }
  print('swaps: $swaps');
}
