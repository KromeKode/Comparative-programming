import 'dart:io';

int Function() makeCounter(int start, int step) {
  var next = start; // captured by the closure, and kept alive by it
  return () {
    final value = next;
    next += step;
    return value;
  };
}

void main() {
  final counters = <String, int Function()>{};
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    switch (line.trim().split(RegExp(r'\s+'))) {
      case ['new', final name, final start, final step]:
        counters[name] = makeCounter(int.parse(start), int.parse(step));
      case ['next', final name]:
        final counter = counters[name];
        print(
          counter == null ? '$name: no such counter' : '$name: ${counter()}',
        );
    }
  }
}
