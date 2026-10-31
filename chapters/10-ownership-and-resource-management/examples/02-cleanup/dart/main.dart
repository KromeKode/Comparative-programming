class FileError implements Exception {
  final String message;
  FileError(this.message);

  @override
  String toString() => message;
}

// A pretend file that prints when it opens and closes.
class File {
  final String name;

  File(this.name, {bool fails = false}) {
    if (fails) throw FileError('cannot open $name');
    print('  open $name');
  }

  void close() => print('  close $name');
}

// A finally block runs however its try block ends, so each file gets one.
void runJob(bool outputFails, bool copyFails) {
  final input = File('input');
  try {
    final output = File('output', fails: outputFails);
    try {
      if (copyFails) throw FileError('disk full');
      print('  copy input to output');
    } finally {
      output.close();
    }
  } finally {
    input.close();
  }
}

void main() {
  final jobs = [
    ('everything works', false, false),
    ('opening output fails', true, false),
    ('copying fails', false, true),
  ];
  for (final (i, (title, outputFails, copyFails)) in jobs.indexed) {
    final number = i + 1;
    print('job $number: $title');
    try {
      runJob(outputFails, copyFails);
      print('  job $number done');
    } on FileError catch (error) {
      print('  job $number failed: $error');
    }
  }
}
