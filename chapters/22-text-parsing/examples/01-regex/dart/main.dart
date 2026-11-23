// Reads a web server's log, one request on each line, such as
//   2026-11-23 14:05:12 GET /books/42 200 12ms
// and prints, for each method and route, how many requests there were, how many failed,
// and the slowest. A route is the path with each "/" and the digits after it written
// "/:id", so that /books/42 and /books/7 count together. A line that isn't a request
// gets a message on stderr, and the exit status is then 1.
import 'dart:convert';
import 'dart:io';
import 'dart:math';

// ^ and $, because firstMatch finds a match anywhere in the line. Without multiLine, $
// matches only at the very end. In Dart, \d is only [0-9], but this says so.
final line = RegExp(
  r'^(?<date>[0-9]{4}-[0-9]{2}-[0-9]{2}) (?<time>[0-9]{2}:[0-9]{2}:[0-9]{2})'
  r' (?<method>GET|POST|PUT|DELETE) (?<path>/[^ ]*)'
  r' (?<status>[0-9]{3}) (?<ms>[0-9]{1,9})ms$',
);
final id = RegExp(r'/[0-9]+');

class Route {
  int requests = 0;
  int failed = 0;
  int slowest = 0;
}

Future<void> main() async {
  // A map literal keeps its keys in the order they went in.
  final routes = <String, Route>{};
  var status = 0;
  // LineSplitter would also split at a "\r" on its own, so this splits at "\n" only.
  final lines = (await utf8.decodeStream(stdin)).split('\n');
  // The last is empty when the input ended with a newline, or was empty.
  if (lines.last == '') lines.removeLast();
  for (final (index, text) in lines.indexed) {
    final match = line.firstMatch(
      text.endsWith('\r') ? text.substring(0, text.length - 1) : text,
    );
    if (match == null) {
      stderr.writeln('logscan: line ${index + 1}: not a log line');
      status = 1;
      continue;
    }
    // namedGroup returns a String?, which is null only for a group that didn't take
    // part in the match.
    String group(String name) => match.namedGroup(name)!;
    final key = '${group('method')} ${group('path').replaceAll(id, '/:id')}';
    final route = routes.putIfAbsent(key, Route.new);
    route.requests++;
    if (int.parse(group('status')) >= 400) route.failed++;
    route.slowest = max(route.slowest, int.parse(group('ms')));
  }
  for (final MapEntry(:key, value: route) in routes.entries) {
    final plural = route.requests == 1 ? '' : 's';
    print(
      '$key: ${route.requests} request$plural, ${route.failed} failed, '
      'slowest ${route.slowest}ms',
    );
  }
  exitCode = status;
}
