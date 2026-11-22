import 'dart:io';

class Tracer {
  bool on = false;
  int depth = 0;
  int calls = 0;

  void say(String text) {
    if (on) print('  ' * depth + text);
  }
}

final tracer = Tracer();

// traced runs body as one traced call. Dart can't find a function's name
// while the program runs, so each function passes its own.
int traced(String name, List<int> args, int Function() body) {
  tracer.calls++;
  tracer.say('$name(${args.join(', ')})');
  tracer.depth++;
  final int result;
  try {
    result = body();
  } finally {
    tracer.depth--;
  }
  tracer.say('-> $result');
  return result;
}

// The recursive calls in each body go through the function, and so through
// traced, again.
int fib(int n) => traced('fib', [n], () => n < 2 ? n : fib(n - 1) + fib(n - 2));

int gcd(int a, int b) =>
    traced('gcd', [a, b], () => b == 0 ? a : gcd(b, a % b));

int power(int base, int exponent) => traced('power', [base, exponent], () {
  if (exponent == 0) return 1;
  if (exponent.isEven) {
    final half = power(base, exponent ~/ 2);
    return half * half;
  }
  return base * power(base, exponent - 1);
});

final functions = <String, Function>{'fib': fib, 'gcd': gcd, 'power': power};

// Function types are kept when the program runs, so a type test can tell how
// many ints a function takes.
int arity(Function function) {
  if (function is int Function(int)) return 1;
  if (function is int Function(int, int)) return 2;
  throw ArgumentError('not a traced function');
}

String plural(int count, String word) => '$count $word${count == 1 ? '' : 's'}';

void run(String line) {
  final words = line.trim().split(RegExp(r'\s+'));
  final [command, name, ...rest] = words;
  final args = rest.map(int.parse).toList();
  if (command != 'trace' && command != 'count') {
    print('unknown command: $command');
    return;
  }
  final function = functions[name];
  if (function == null) {
    print('unknown function: $name');
    return;
  }
  final takes = arity(function);
  if (args.length != takes) {
    print('$name takes ${plural(takes, 'argument')}, got ${args.length}');
    return;
  }
  tracer
    ..on = command == 'trace'
    ..depth = 0
    ..calls = 0;
  // Function.apply calls a function with a list of positional arguments.
  final result = Function.apply(function, args) as int;
  print(
    '$name(${args.join(', ')}) = $result in ${plural(tracer.calls, 'call')}',
  );
}

void main() {
  for (String? line; (line = stdin.readLineSync()) != null;) {
    if (line!.trim().isNotEmpty) run(line);
  }
}
