// Removes repeated lines from each file named on the command line, keeping the first of
// each, and rewrites the file in place. It writes a new file next to the old one and
// renames it over the old one, so that a crash halfway leaves the old file whole.
import 'dart:io';

/// Rewrites the file without its repeated lines, and returns how many there were.
int dedup(String path) {
  // readAsStringSync decodes UTF-8, and throws if the bytes aren't UTF-8.
  final lines = File(path).readAsStringSync().split('\n');
  if (lines.last == '')
    lines.removeLast(); // it ended with a newline, or was empty
  final kept = lines
      .toSet()
      .toList(); // a Set keeps the first of each, in order
  final removed = lines.length - kept.length;
  if (removed == 0) return 0;

  // Dart can't make a file with a fresh name, so the new file is FILE.tmp, and
  // exclusive: true fails if that is already there rather than writing over it.
  final temporary = File('$path.tmp')..createSync(exclusive: true);
  try {
    final out = temporary.openSync(mode: FileMode.writeOnly);
    try {
      out.writeStringSync(kept.map((line) => '$line\n').join());
      out.flushSync(); // onto the disk before the rename
    } finally {
      out.closeSync();
    }
    // dart:io can't set a file's permissions, so the new file gets the usual ones.
    temporary.renameSync(path);
  } catch (_) {
    if (temporary.existsSync()) temporary.deleteSync();
    rethrow;
  }
  return removed;
}

void main(List<String> arguments) {
  if (arguments.isEmpty) {
    stderr.writeln('usage: dedup FILE...');
    exit(2);
  }
  for (final path in arguments) {
    final int removed;
    try {
      removed = dedup(path);
    } on FileSystemException catch (error) {
      // osError has the system's message, such as "No such file or directory".
      stderr.writeln(
        'dedup: $path: ${error.osError?.message ?? error.message}',
      );
      exitCode = 1;
      continue;
    }
    print('$path: removed $removed line${removed == 1 ? '' : 's'}');
  }
}
