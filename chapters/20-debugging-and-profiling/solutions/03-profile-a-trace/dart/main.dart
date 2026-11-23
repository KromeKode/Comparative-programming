// Reads a trace of function calls and prints a flat profile: for each function, how
// many calls it got, its total time, and its self time. See ../README.md.
import 'dart:io';

class FunctionStats {
  var calls = 0;
  var total = 0;
  var self = 0;
  var running = 0; // how many of its calls are running now
  var outermostStart = 0; // when the outermost of them started
}

class Call {
  Call(this.name, this.start);

  final String name;
  final int start;
  var inCallees = 0; // time spent in the functions it called
}

class TraceError implements Exception {
  TraceError(this.message);

  final String message;
}

final timePattern = RegExp(r'^[0-9]{1,12}$');
final namePattern = RegExp(r'^[!-~]+$');

Map<String, FunctionStats> profile(Iterable<String> lines) {
  final functions = <String, FunctionStats>{};
  final stack = <Call>[];
  var now = 0;
  var number = 0;
  for (final line in lines) {
    number++;
    final fields = line
        .split(RegExp(r'[ \t]+'))
        .where((f) => f.isNotEmpty)
        .toList();
    if (fields.isEmpty) continue;
    if (fields.length != 3 ||
        !timePattern.hasMatch(fields[0]) ||
        !namePattern.hasMatch(fields[2]) ||
        (fields[1] != 'enter' && fields[1] != 'exit')) {
      throw TraceError(
        'line $number: expected TIME enter NAME or TIME exit NAME',
      );
    }
    final time = int.parse(fields[0]);
    final name = fields[2];
    if (time < now) {
      throw TraceError('line $number: time goes back from $now to $time');
    }
    now = time;

    if (fields[1] == 'enter') {
      final f = functions.putIfAbsent(name, FunctionStats.new);
      f.calls++;
      if (f.running == 0) f.outermostStart = time;
      f.running++;
      stack.add(Call(name, time));
      continue;
    }

    if (stack.isEmpty) {
      throw TraceError('line $number: exit from $name, but nothing is running');
    }
    if (stack.last.name != name) {
      throw TraceError(
        'line $number: exit from $name, but the innermost call is to ${stack.last.name}',
      );
    }
    final call = stack.removeLast();
    final duration = time - call.start;
    final f = functions[name]!;
    f.self += duration - call.inCallees;
    f.running--;
    // A recursive call's time is already in the total of the call around it.
    if (f.running == 0) f.total += time - f.outermostStart;
    if (stack.isNotEmpty) stack.last.inCallees += duration;
  }
  if (stack.isNotEmpty) {
    throw TraceError('end of trace: ${stack.last.name} is still running');
  }
  return functions;
}

Iterable<String> readLines() sync* {
  for (String? line; (line = stdin.readLineSync()) != null;) {
    yield line!;
  }
}

void main() {
  final Map<String, FunctionStats> functions;
  try {
    functions = profile(readLines());
  } on TraceError catch (error) {
    print('error: ${error.message}');
    return;
  }
  if (functions.isEmpty) print('no calls');
  final names = functions.keys.toList()
    ..sort((a, b) {
      final bySelf = functions[b]!.self.compareTo(functions[a]!.self);
      return bySelf != 0 ? bySelf : a.compareTo(b);
    });
  for (final name in names) {
    final f = functions[name]!;
    print('$name ${f.calls} ${f.total} ${f.self}');
  }
}
