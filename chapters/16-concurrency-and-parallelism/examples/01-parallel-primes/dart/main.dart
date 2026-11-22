import 'dart:isolate';

const limit = 200000;
const workers = 4;

bool isPrime(int n) {
  if (n < 2) return false;
  for (var d = 2; d * d <= n; d++) {
    if (n % d == 0) return false;
  }
  return true;
}

int countPrimes(int start, int end) {
  var count = 0;
  for (var n = start; n < end; n++) {
    if (isPrime(n)) count++;
  }
  return count;
}

Future<void> main() async {
  print('counting primes below $limit with $workers workers');
  final jobs = [
    for (var i = 0; i < workers; i++)
      (start: limit * i ~/ workers, end: limit * (i + 1) ~/ workers),
  ];

  // Isolate.run starts an isolate, a Dart thread with its own memory, runs
  // the function there, and returns its result as a Future. Future.wait
  // waits for all of them, and keeps the results in order.
  final counts = await Future.wait([
    for (final job in jobs) Isolate.run(() => countPrimes(job.start, job.end)),
  ]);
  for (final (i, job) in jobs.indexed) {
    print('worker $i: [${job.start}, ${job.end}) has ${counts[i]} primes');
  }
  print('total: ${counts.reduce((a, b) => a + b)} primes');
}
