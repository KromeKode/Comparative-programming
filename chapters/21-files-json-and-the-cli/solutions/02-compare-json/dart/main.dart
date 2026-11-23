// Prints how one JSON file differs from another: what was removed, what was added, and
// what changed. ../README.md says what it should print.
import 'dart:convert';
import 'dart:io';
import 'dart:math';

const usage = 'usage: jsondiff OLD NEW';
final identifier = RegExp(r'^[A-Za-z_][A-Za-z0-9_]*$');

/// The keys in order of their characters' codes. jsonDecode's maps keep the order of
/// the file.
List<String> sortedKeys(Iterable<String> keys) => keys.toSet().toList()..sort();

String compact(Object? value) => switch (value) {
  Map<String, Object?> map =>
    '{${sortedKeys(map.keys).map((key) => '${jsonEncode(key)}:${compact(map[key])}').join(',')}}',
  List<Object?> list => '[${list.map(compact).join(',')}]',
  _ => jsonEncode(value),
};

/// The part of a path that leads to a key: .name, or ["first name"].
String step(String key) =>
    identifier.hasMatch(key) ? '.$key' : '[${jsonEncode(key)}]';

/// Adds a line to out for each difference between before and after.
void diff(String path, Object? before, Object? after, List<String> out) {
  switch ((before, after)) {
    case (Map<String, Object?> before, Map<String, Object?> after):
      for (final key in sortedKeys([...before.keys, ...after.keys])) {
        final here = path + step(key);
        if (!after.containsKey(key)) {
          out.add('- $here: ${compact(before[key])}');
        } else if (!before.containsKey(key)) {
          out.add('+ $here: ${compact(after[key])}');
        } else {
          diff(here, before[key], after[key], out);
        }
      }
    case (List<Object?> before, List<Object?> after):
      for (var i = 0; i < max(before.length, after.length); i++) {
        final here = '$path[$i]';
        if (i >= after.length) {
          out.add('- $here: ${compact(before[i])}');
        } else if (i >= before.length) {
          out.add('+ $here: ${compact(after[i])}');
        } else {
          diff(here, before[i], after[i], out);
        }
      }
    // Two maps or two lists went the ways above, so at least one of before and after
    // is a String, int, bool, or null, which == compares by value.
    case _ when before != after:
      out.add('~ $path: ${compact(before)} -> ${compact(after)}');
  }
}

void main(List<String> args) {
  if (args.length != 2) {
    stderr.writeln(usage);
    exitCode = 2;
    return;
  }
  final values = <Object?>[];
  for (final path in args) {
    try {
      // readAsStringSync throws a FileSystemException if the file can't be read, or
      // isn't UTF-8; jsonDecode throws a FormatException if it isn't JSON.
      values.add(jsonDecode(File(path).readAsStringSync()));
    } on FileSystemException catch (error) {
      // osError is the system's error, which a file that isn't UTF-8 doesn't have.
      final reason = error.osError?.message ?? 'not valid JSON';
      stderr.writeln('jsondiff: $path: $reason');
      exitCode = 2;
      return;
    } on FormatException {
      stderr.writeln('jsondiff: $path: not valid JSON');
      exitCode = 2;
      return;
    }
  }
  final lines = <String>[];
  diff(r'$', values[0], values[1], lines);
  stdout.write(lines.map((line) => '$line\n').join());
  exitCode = lines.isEmpty ? 0 : 1;
}
