// Exercise 18.2: build order. The order to build modules in, and what to rebuild.
import 'dart:collection';
import 'dart:io';

final namePattern = RegExp(r'^[A-Za-z0-9_./-]+$');

typedef Module = ({int line, List<String> imports});

List<String> words(String text) =>
    text.split(RegExp(r'\s+')).where((word) => word.isNotEmpty).toList();

List<String> run(List<String> lines) {
  final out = <String>[];
  // A Dart map literal is a LinkedHashMap, which keeps its keys in the order they
  // were added: the order of definition.
  final modules = <String, Module>{};
  final queries = <(int, String)>[];
  for (final (index, line) in lines.indexed) {
    final number = index + 1;
    final colon = line.indexOf(':');
    if (colon >= 0) {
      final name = line.substring(0, colon).trim();
      final imports = words(line.substring(colon + 1));
      if (!namePattern.hasMatch(name) || !imports.every(namePattern.hasMatch)) {
        out.add('error: line $number: a module is NAME: IMPORTS');
        return out;
      }
      if (modules[name] case final earlier?) {
        out.add(
          'error: line $number: $name is already defined on line ${earlier.line}',
        );
        return out;
      }
      // toSet drops repeats and keeps the first of each, in order.
      modules[name] = (line: number, imports: imports.toSet().toList());
      continue;
    }
    switch (words(line)) {
      case []:
        break;
      case ['rebuild', final name]:
        queries.add((number, name));
      default:
        out.add('error: line $number: expected NAME: IMPORTS or rebuild NAME');
        return out;
    }
  }

  final users = {for (final name in modules.keys) name: <String>[]};
  final waiting = <String, int>{};
  for (final MapEntry(key: name, value: module) in modules.entries) {
    for (final imported in module.imports) {
      if (!modules.containsKey(imported)) {
        out.add(
          "error: line ${module.line}: $name imports $imported, which isn't defined",
        );
        return out;
      }
      users[imported]!.add(name);
    }
    waiting[name] = module.imports.length;
  }

  // Kahn's algorithm: a module is ready once everything it imports is built, and
  // of the ready modules, the first in alphabetical order goes next. A
  // SplayTreeSet keeps its strings sorted, so its first is the one.
  final ready = SplayTreeSet.of(
    modules.keys.where((name) => waiting[name] == 0),
  );
  final order = <String>[];
  while (ready.isNotEmpty) {
    final name = ready.first;
    ready.remove(name);
    order.add(name);
    for (final user in users[name]!) {
      waiting[user] = waiting[user]! - 1;
      if (waiting[user] == 0) ready.add(user);
    }
  }

  if (order.length < modules.length) {
    // Every module left over imports another one left over, so following first
    // imports from the first module left over must come back to a module.
    bool isLeft(String name) => waiting[name]! > 0;
    final path = [(modules.keys.where(isLeft).toList()..sort()).first];
    final seen = {path.first: 0};
    while (true) {
      final next = modules[path.last]!.imports.firstWhere(isLeft);
      if (seen[next] case final at?) {
        final cycle = [...path.sublist(at), next];
        out.add('error: import cycle: ${cycle.join(' -> ')}');
        return out;
      }
      seen[next] = path.length;
      path.add(next);
    }
  }

  out.add(
    'build order: ${order.isEmpty ? 'nothing to build' : order.join(', ')}',
  );
  for (final (number, name) in queries) {
    if (!modules.containsKey(name)) {
      out.add('error: line $number: no module called $name');
      continue;
    }
    // Everything that imports the changed module, directly or not, comes after it
    // in the build order, so one pass through the order finds them all. A Dart set
    // literal keeps the order things were added in, which here is build order.
    final affected = {name};
    for (final module in order) {
      if (modules[module]!.imports.any(affected.contains)) affected.add(module);
    }
    out.add('rebuild $name: ${affected.join(', ')}');
  }
  return out;
}

void main() {
  final lines = <String>[];
  for (String? line; (line = stdin.readLineSync()) != null;) {
    lines.add(line!);
  }
  run(lines).forEach(print);
}
