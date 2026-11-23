// Prints the lines that contain a pattern, from each file named on the command line, or
// from stdin. ../README.md says what it should print.
import 'dart:convert';
import 'dart:io';
import 'dart:typed_data';

const usage = 'usage: search [-icnv] PATTERN [FILE...]';
const stdinName = '(standard input)';

Never usageError(String message) {
  stderr.writeln('search: $message\n$usage');
  exit(2);
}

/// A copy of the bytes with A to Z made a to z, and every other byte as it was.
Uint8List lower(List<int> bytes) => Uint8List.fromList([
  for (final byte in bytes) byte >= 0x41 && byte <= 0x5a ? byte + 0x20 : byte,
]);

/// Whether pattern is somewhere in bytes.
bool contains(List<int> bytes, List<int> pattern) {
  outer:
  for (var i = 0; i + pattern.length <= bytes.length; i++) {
    for (var j = 0; j < pattern.length; j++) {
      if (bytes[i + j] != pattern[j]) continue outer;
    }
    return true;
  }
  return false;
}

/// Writes the lines of data that are selected, and returns how many there are.
int search(
  Uint8List data,
  String? name,
  List<int> pattern,
  Set<String> options,
) {
  final out = BytesBuilder(copy: false);
  final prefix = name == null ? '' : '$name:';
  if (options.contains('i')) pattern = lower(pattern);
  var count = 0, number = 0;
  for (var start = 0; start < data.length;) {
    var end = data.indexOf(0x0a, start); // 0x0a is '\n'
    if (end == -1) end = data.length; // a last line without a newline
    // sublistView shares data's bytes, without copying them.
    final line = Uint8List.sublistView(data, start, end);
    number++;
    start = end + 1;
    final found = contains(options.contains('i') ? lower(line) : line, pattern);
    if (found == options.contains('v')) continue;
    count++;
    if (!options.contains('c')) {
      out.add(utf8.encode('$prefix${options.contains('n') ? '$number:' : ''}'));
      out.add(line);
      out.addByte(0x0a);
    }
  }
  if (options.contains('c')) out.add(utf8.encode('$prefix$count\n'));
  stdout.add(out.takeBytes());
  return count;
}

Future<void> main(List<String> arguments) async {
  final args = [...arguments];
  final options = <String>{};
  // Options come first; the first argument that isn't one is the pattern.
  while (args.isNotEmpty && args.first.startsWith('-') && args.first != '-') {
    final arg = args.removeAt(0);
    if (arg == '--') break;
    for (final letter in arg.substring(1).split('')) {
      if (!'icnv'.contains(letter)) usageError('unknown option -$letter');
      options.add(letter);
    }
  }
  if (args.isEmpty) usageError('no pattern');
  final pattern = utf8.encode(args.first);
  final files = args.length > 1 ? args.sublist(1) : ['-'];

  var selected = 0, status = 0;
  for (final file in files) {
    final Uint8List data;
    try {
      if (file == '-') {
        // stdin is a stream of chunks of bytes, which come as they arrive.
        final builder = BytesBuilder(copy: false);
        await for (final chunk in stdin) {
          builder.add(chunk);
        }
        data = builder.takeBytes();
      } else {
        data = File(file).readAsBytesSync(); // bytes, with nothing decoded
      }
    } on FileSystemException catch (error) {
      await stdout.flush(); // what came before, before the error
      stderr.writeln(
        'search: $file: ${error.osError?.message ?? error.message}',
      );
      status = 2;
      continue;
    }
    final name = files.length == 1 ? null : (file == '-' ? stdinName : file);
    selected += search(data, name, pattern, options);
  }
  // exitCode, not exit(): exit would end the program before stdout had written it all.
  exitCode = status == 0 && selected == 0 ? 1 : status;
}
