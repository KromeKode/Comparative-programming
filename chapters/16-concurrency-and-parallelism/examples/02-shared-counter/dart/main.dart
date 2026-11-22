import 'dart:isolate';

const isolates = 4;
const times = 100000;

// Each isolate has its own copy of every top-level variable, starting from
// its initial value. Isolates share no memory, so they can't race on it.
var count = 0;

int addMany() {
  for (var i = 0; i < times; i++) {
    count += 1;
  }
  return count;
}

Future<void> main() async {
  print('$isolates isolates add 1, $times times each');
  final results = await Future.wait([
    for (var i = 0; i < isolates; i++) Isolate.run(addMany),
  ]);
  print('each isolate counted: ${results.join(', ')}');
  print("main's count: $count");
  // The only way to combine their work is to send the results back.
  print('added up: ${results.reduce((a, b) => a + b)}');
}
