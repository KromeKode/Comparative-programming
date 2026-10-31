import 'dart:io';

// Returns the pair with the smallest j, and for that j the smallest i.
(int, int)? twoSum(int target, List<int> numbers) {
  // Each number seen so far, mapped to the first index where it appeared.
  final firstIndex = <int, int>{};
  for (var j = 0; j < numbers.length; j++) {
    final i = firstIndex[target - numbers[j]];
    if (i != null) return (i, j);
    firstIndex.putIfAbsent(numbers[j], () => j);
  }
  return null;
}

void main() {
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    final colon = line.indexOf(':');
    if (colon == -1) continue; // a blank line
    final target = int.parse(line.substring(0, colon).trim());
    final words = line.substring(colon + 1).trim().split(RegExp(r'\s+'));
    final numbers = [
      for (final word in words)
        if (word.isNotEmpty) int.parse(word),
    ];
    final pair = twoSum(target, numbers);
    print(pair == null ? 'none' : '${pair.$1} ${pair.$2}');
  }
}
