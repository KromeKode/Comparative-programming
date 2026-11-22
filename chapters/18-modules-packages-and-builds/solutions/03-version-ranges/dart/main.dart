// Exercise 18.3: version ranges. The highest version that each range allows.
//
// The versions and ranges are semver.dart's business; this file reads the input
// and prints the answers.
import 'dart:io';

import 'semver.dart';

void main() {
  final lines = <String>[];
  for (String? line; (line = stdin.readLineSync()) != null;) {
    lines.add(line!);
  }
  if (lines.isEmpty) return;
  final versions = <Version>[];
  for (final word
      in lines[0].split(RegExp(r'\s+')).where((w) => w.isNotEmpty)) {
    final version = parseVersion(word);
    if (version == null) {
      print("error: line 1: $word isn't a version");
      return;
    }
    versions.add(version);
  }
  if (versions.isEmpty) {
    print('error: line 1 must list the versions');
    return;
  }
  for (final (index, raw) in lines.indexed.skip(1)) {
    final line = raw.trim();
    if (line.isEmpty) continue;
    try {
      final best = parseRange(line).highest(versions);
      print('$line -> ${best ?? 'none'}');
    } on FormatException catch (error) {
      print('error: line ${index + 1}: ${error.message}');
    }
  }
}
