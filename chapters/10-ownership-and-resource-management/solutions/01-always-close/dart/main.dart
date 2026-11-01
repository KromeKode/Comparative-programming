import 'dart:io';

const maxOpen = 2;
const maxLines = 8;

class DiskError implements Exception {
  final String message;
  DiskError(this.message);

  @override
  String toString() => message;
}

class FileData {
  List<String> lines = [];
  bool locked = false;
}

class Disk {
  final files = <String, FileData>{};
  int openCount = 0;

  int linesUsed() => files.values.fold(0, (sum, f) => sum + f.lines.length);

  // Returns an open Handle. The caller must close it, in a finally block.
  Handle open(String name, {bool writing = false}) {
    if (openCount == maxOpen) throw DiskError('too many open files');
    final file = files[name];
    if (writing) {
      if (file != null && file.locked) {
        throw DiskError('"$name" is read-only');
      }
      files.putIfAbsent(name, FileData.new).lines = [];
    } else if (file == null) {
      throw DiskError('no file "$name"');
    }
    openCount++;
    print('open $name');
    return Handle(this, name);
  }
}

class Handle {
  final Disk disk;
  final String name;
  Handle(this.disk, this.name);

  List<String> lines() => List.of(disk.files[name]!.lines);

  void write(String line) {
    if (disk.linesUsed() == maxLines) throw DiskError('disk full');
    disk.files[name]!.lines.add(line);
  }

  void close() {
    disk.openCount--;
    print('close $name');
  }
}

String lines(int count) => count == 1 ? '1 line' : '$count lines';

void writeFile(Disk disk, String name, List<String> words) {
  final target = disk.open(name, writing: true);
  try {
    for (final word in words) {
      target.write(word);
    }
  } finally {
    target.close();
  }
  print('wrote ${lines(words.length)}');
}

void catFile(Disk disk, String name) {
  final source = disk.open(name);
  try {
    for (final line in source.lines()) {
      print('  $line');
    }
  } finally {
    source.close();
  }
}

void copyFile(Disk disk, String from, String to) {
  if (from == to) throw DiskError('cannot copy "$from" onto itself');
  var copied = 0;
  final source = disk.open(from);
  try {
    final target = disk.open(to, writing: true);
    try {
      for (final line in source.lines()) {
        target.write(line);
        copied++;
      }
    } finally {
      target.close();
    }
  } finally {
    source.close();
  }
  print('copied ${lines(copied)}');
}

void lockFile(Disk disk, String name) {
  final file = disk.files[name];
  if (file == null) throw DiskError('no file "$name"');
  file.locked = true;
  print('locked $name');
}

void takes(String command, int want, List<String> args) {
  if (args.length != want) {
    final noun = want == 1 ? 'name' : 'names';
    throw DiskError('$command takes $want $noun, got ${args.length}');
  }
}

void run(Disk disk, String command, List<String> args) {
  switch (command) {
    case 'write':
      if (args.isEmpty) throw DiskError('write needs a file name');
      writeFile(disk, args[0], args.sublist(1));
    case 'cat':
      takes(command, 1, args);
      catFile(disk, args[0]);
    case 'copy':
      takes(command, 2, args);
      copyFile(disk, args[0], args[1]);
    case 'lock':
      takes(command, 1, args);
      lockFile(disk, args[0]);
    default:
      throw DiskError('unknown command "$command"');
  }
}

void main() {
  final disk = Disk();
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    final parts = line.split(RegExp(r'\s+')).where((p) => p.isNotEmpty);
    if (parts.isEmpty) continue;
    try {
      run(disk, parts.first, parts.skip(1).toList());
    } on DiskError catch (error) {
      print('error: $error');
    }
  }
  print('files still open: ${disk.openCount}');
}
