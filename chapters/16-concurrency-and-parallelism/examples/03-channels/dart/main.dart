import 'dart:isolate';

const count = 1000;
const workers = 3;

// Runs in a worker isolate. Isolates share no memory, and talk through
// ports: a message sent to a SendPort arrives, copied, at its ReceivePort.
// The worker sends back a port for its jobs, then the square of each job,
// and stops at null.
void work(SendPort replies) {
  final jobs = ReceivePort();
  replies.send(jobs.sendPort);
  jobs.listen((job) {
    if (job == null) {
      jobs.close(); // with nothing left to listen to, the isolate ends
    } else {
      replies.send((job as int) * job);
    }
  });
}

Future<void> main() async {
  // Only the main isolate hands out jobs, so no two workers get the same one.
  var next = 1, received = 0, sum = 0;

  Future<void> drive() async {
    final replies = ReceivePort();
    await Isolate.spawn(work, replies.sendPort);
    late SendPort jobs;
    // A ReceivePort is a Stream, and leaving the loop closes it.
    await for (final message in replies) {
      if (message is SendPort) {
        jobs = message;
      } else {
        received++;
        sum += message as int;
      }
      if (next > count) {
        jobs.send(null);
        break;
      }
      jobs.send(next++);
    }
  }

  await Future.wait([for (var i = 0; i < workers; i++) drive()]);
  print('$workers workers squared 1 to $count');
  print('results: $received');
  print('sum: $sum');
}
