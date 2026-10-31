import 'dart:convert';
import 'dart:io';

Future<void> main() async {
  final input = await stdin.transform(utf8.decoder).join();

  // A map literal keeps keys in the order they were added, which is the order the
  // groups must be printed in.
  final groups = <String, List<String>>{};
  for (final raw in input.split(RegExp(r'\s+'))) {
    if (raw.isEmpty) continue;
    final word = raw.toLowerCase();
    // Anagrams have the same letters once sorted.
    final key = (word.split('')..sort()).join();
    groups.putIfAbsent(key, () => []).add(word);
  }

  for (final group in groups.values) {
    print(group.join(' '));
  }
}
