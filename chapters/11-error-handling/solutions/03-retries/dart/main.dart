import 'dart:io';
import 'dart:math';

const maxAttempts = 3;

// A sealed class, so the switches below must handle every kind.
sealed class DownloadError implements Exception {
  const DownloadError();

  String get message => switch (this) {
    TimedOut() => 'timed out',
    ServerBusy() => 'server busy',
    NotFound() => 'not found',
    LoginRefused() => 'login refused',
    GaveUp(:final attempts, :final last) =>
      'gave up after $attempts attempts: ${last.message}',
  };

  // Try the same file again?
  bool get retry => switch (this) {
    TimedOut() || ServerBusy() => true,
    NotFound() || LoginRefused() || GaveUp() => false,
  };

  @override
  String toString() => message;
}

class TimedOut extends DownloadError {
  const TimedOut();
}

class ServerBusy extends DownloadError {
  const ServerBusy();
}

class NotFound extends DownloadError {
  const NotFound();
}

class LoginRefused extends DownloadError {
  const LoginRefused();
}

class GaveUp extends DownloadError {
  final int attempts;
  final DownloadError last;
  const GaveUp(this.attempts, this.last);
}

// The pretend server: throw the error for this attempt's answer, if any.
void download(List<String> answers, int attempt) {
  switch (answers[min(attempt, answers.length) - 1]) {
    case 'timeout':
      throw const TimedOut();
    case 'busy':
      throw const ServerBusy();
    case 'missing':
      throw const NotFound();
    case 'auth':
      throw const LoginRefused();
  }
}

// Return the number of attempts it took, or throw the error that stopped it.
int downloadWithRetries(String name, List<String> answers) {
  var wait = 1;
  for (var attempt = 1; ; attempt++) {
    try {
      download(answers, attempt);
      return attempt;
    } on DownloadError catch (error) {
      if (!error.retry) rethrow;
      if (attempt == maxAttempts) throw GaveUp(attempt, error);
      print('$name: $error, retrying in ${wait}s');
      wait *= 2;
    }
  }
}

void main() {
  final jobs = <List<String>>[];
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    final words = line.trim().split(RegExp(r' +'));
    if (words.first.isNotEmpty) jobs.add(words);
  }

  var (downloaded, skipped, failed, notTried) = (0, 0, 0, 0);
  // The label lets the break below leave the loop, not just the switch.
  batch:
  for (var index = 0; index < jobs.length; index++) {
    final [name, ...answers] = jobs[index];
    try {
      final attempts = downloadWithRetries(name, answers);
      print('$name: ok after $attempts attempt${attempts == 1 ? '' : 's'}');
      downloaded++;
    } on DownloadError catch (error) {
      switch (error) {
        case GaveUp():
          print('$name: failed: $error');
          failed++;
        case LoginRefused():
          print('$name: stopped: $error');
          failed++;
          notTried = jobs.length - index - 1;
          break batch;
        case TimedOut() || ServerBusy() || NotFound():
          print('$name: skipped: $error');
          skipped++;
      }
    }
  }
  print(
    'downloaded $downloaded, skipped $skipped, failed $failed, '
    'not tried $notTried',
  );
}
