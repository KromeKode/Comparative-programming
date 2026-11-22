// Completes on a later turn of the event loop. A task that awaits it goes to
// the back of the line, behind everything that is ready to run.
Future<void> pause() => Future(() {});

Future<int> task(String name, int steps) async {
  for (var step = 1; step <= steps; step++) {
    print('$name: step $step');
    // await is the only place a task can stop and let another one run.
    await pause();
  }
  print('$name: done');
  return steps;
}

class Counter {
  int value = 0;
}

Future<void> addAtOnce(Counter count) async {
  count.value += 1; // no await, so no other task can run in the middle
}

Future<void> addWithAwait(Counter count) async {
  final value = count.value;
  await pause(); // every other task reads the old value here
  count.value = value + 1;
}

Future<void> main() async {
  print('three tasks, each pausing at every await');
  // Calling an async function runs it up to its first await, and returns a
  // Future. Future.wait waits for all of them, and keeps the results in order.
  final results = await Future.wait([task('a', 2), task('b', 3), task('c', 1)]);
  print('results: ${results.join(', ')}');

  print('4 tasks add 1 to a shared count');
  final atOnce = Counter();
  await Future.wait([for (var i = 0; i < 4; i++) addAtOnce(atOnce)]);
  print('with no await between reading and writing: ${atOnce.value}');
  final withAwait = Counter();
  await Future.wait([for (var i = 0; i < 4; i++) addWithAwait(withAwait)]);
  print('with an await between reading and writing: ${withAwait.value}');
}
