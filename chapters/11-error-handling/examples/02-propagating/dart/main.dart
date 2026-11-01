// Pretend files, so the example does not touch the disk.
const files = {'good.txt': '1\n2\n3', 'bad.txt': '4\nfive\n6'};

// One exception class per kind of failure, so callers can catch just one.
class NotFound implements Exception {
  final String message;
  NotFound(this.message);
  @override
  String toString() => message;
}

class NotANumber implements Exception {
  final String message;
  NotANumber(this.message);
  @override
  String toString() => message;
}

// Dart exceptions have no built-in cause, so this class keeps its own.
class BadLine implements Exception {
  final String message;
  final Object cause;
  BadLine(this.message, this.cause);
  @override
  String toString() => message;
}

String readFile(String name) =>
    files[name] ?? (throw NotFound('$name: no such file'));

int parseNumber(String text) =>
    int.tryParse(text) ?? (throw NotANumber('"$text" is not a number'));

int sumFile(String name) {
  final text = readFile(name); // a NotFound passes straight through
  var total = 0;
  final lines = text.split('\n');
  for (var index = 0; index < lines.length; index++) {
    try {
      total += parseNumber(lines[index]);
    } on NotANumber catch (error) {
      throw BadLine('$name: line ${index + 1}', error);
    }
  }
  return total;
}

// A missing file counts as 0, but any other error stops the sum.
int sumAll(List<String> names) {
  var total = 0;
  for (final name in names) {
    try {
      total += sumFile(name);
    } on NotFound {
      // skip it
    }
  }
  return total;
}

// Join an exception's message with the messages of its causes.
String describe(Object error) =>
    error is BadLine ? '$error: ${describe(error.cause)}' : '$error';

void show(String label, int Function() run) {
  try {
    print('  $label -> ${run()}');
  } on Exception catch (error) {
    print('  $label -> error: ${describe(error)}');
  }
}

void main() {
  print('one file at a time:');
  for (final name in ['good.txt', 'bad.txt', 'missing.txt']) {
    show(name, () => sumFile(name));
  }
  print('several files, where a missing file counts as 0:');
  show('good.txt + missing.txt', () => sumAll(['good.txt', 'missing.txt']));
  show('good.txt + bad.txt', () => sumAll(['good.txt', 'bad.txt']));
}
