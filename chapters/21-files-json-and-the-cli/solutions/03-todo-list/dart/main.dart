// Keeps a to-do list in todo.json, in the current folder: adds tasks, lists them, marks
// them done, and removes them. ../README.md says what it should do.
import 'dart:convert';
import 'dart:io';
import 'dart:math';

const file = 'todo.json';
const version = 1;
const usage = '''usage: todo add TEXT...
       todo list
       todo done ID
       todo remove ID''';
final digits = RegExp(r'^[0-9]+$');

/// A problem with todo.json, which stops the program with status 2.
class Problem implements Exception {
  Problem(this.message);
  final String message;
}

class Task {
  Task(this.id, this.text, this.done);
  final int id;
  final String text;
  bool done;

  /// jsonEncode calls toJson on an object that isn't a number, string, bool, null,
  /// list, or map. A map literal keeps its keys in the order they're written.
  Map<String, Object> toJson() => {'id': id, 'text': text, 'done': done};
}

/// A map pattern matches a map with these keys and maybe others, so the length shows
/// that there are no others. In Dart, true isn't an int, and 1.0 isn't either.
Task taskFromJson(Object? value) => switch (value) {
  Map<String, Object?> task && {
    'id': int id,
    'text': String text,
    'done': bool done,
  } when task.length == 3 && id >= 1 =>
    Task(id, text, done),
  _ => throw Problem('not a to-do list'),
};

/// The tasks in todo.json, or none if there's no todo.json.
List<Task> readTasks() {
  final String text;
  try {
    text = File(file).readAsStringSync();
  } on PathNotFoundException {
    return [];
  } on FileSystemException catch (error) {
    // osError is the system's error, which a file that isn't UTF-8 doesn't have.
    throw Problem(error.osError?.message ?? 'not valid JSON');
  }
  final Object? value;
  try {
    value = jsonDecode(text);
  } on FormatException {
    throw Problem('not valid JSON');
  }
  // A later version of todo may keep anything else in the file, so the version comes
  // first.
  if (value case {'version': num other} when other != version) {
    throw Problem('unknown version $other');
  }
  if (value case Map<String, Object?> list && {
    'version': version,
    'tasks': List<Object?> tasks,
  } when list.length == 2) {
    final result = tasks.map(taskFromJson).toList();
    if (result.map((task) => task.id).toSet().length != result.length) {
      throw Problem('not a to-do list'); // two tasks with one id
    }
    return result;
  }
  throw Problem('not a to-do list');
}

/// Writes the tasks to todo.json. It writes a new file and renames it over the old one,
/// which replaces it in one step, so that a crash leaves the old list or the new one,
/// and never half of one.
void writeTasks(List<Task> tasks) {
  final encoder = JsonEncoder.withIndent('  ');
  final text = '${encoder.convert({'version': version, 'tasks': tasks})}\n';
  final temporary = File('$file.tmp');
  try {
    // flush: onto the disk before the rename
    temporary.writeAsStringSync(text, flush: true);
    temporary.renameSync(file);
  } on FileSystemException catch (error) {
    if (temporary.existsSync()) temporary.deleteSync();
    throw Problem(error.osError?.message ?? error.message);
  }
}

/// Carries out a command whose arguments have been checked, and returns the status.
int run(String command, List<String> args) {
  final tasks = readTasks();
  switch (command) {
    case 'add':
      final id = tasks.map((task) => task.id).fold(0, max) + 1;
      final added = Task(id, args.join(' '), false);
      writeTasks([...tasks, added]);
      print('added ${added.id}: ${added.text}');
      return 0;
    case 'list':
      for (final task in tasks) {
        print('${task.id} [${task.done ? 'x' : ' '}] ${task.text}');
      }
      return 0;
  }
  final id = int.tryParse(args[0]); // null if it's too big for an int
  final i = tasks.indexWhere((task) => task.id == id);
  if (i < 0) {
    stderr.writeln('todo: no task ${args[0]}');
    return 1;
  }
  final found = tasks[i];
  if (command == 'done') {
    found.done = true;
  } else {
    tasks.removeAt(i);
  }
  writeTasks(tasks);
  print('${command == 'done' ? 'done' : 'removed'} ${found.id}: ${found.text}');
  return 0;
}

void main(List<String> arguments) {
  final command = arguments.isEmpty ? '' : arguments.first;
  final args = arguments.skip(1).toList();
  // The command line is checked before todo.json is read.
  final ok = switch (command) {
    'add' => args.isNotEmpty,
    'list' => args.isEmpty,
    'done' || 'remove' => args.length == 1 && digits.hasMatch(args[0]),
    _ => false,
  };
  if (!ok) {
    stderr.writeln(usage);
    exitCode = 2;
    return;
  }
  try {
    exitCode = run(command, args);
  } on Problem catch (problem) {
    stderr.writeln('todo: $file: ${problem.message}');
    exitCode = 2;
  }
}
