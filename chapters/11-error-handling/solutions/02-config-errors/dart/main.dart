import 'dart:io';

class Config {
  final String name;
  final int port;
  final bool debug;
  final int timeout;
  Config(this.name, this.port, this.debug, this.timeout);
}

// The value parsers throw ValueProblem; load wraps it in a BadValue.
class ValueProblem implements Exception {
  final String message;
  ValueProblem(this.message);
}

// Every problem load can find. These are values in a list, not exceptions.
sealed class ConfigError {
  const ConfigError();

  String get message => switch (this) {
    SyntaxProblem(:final line) => 'line $line: expected "key = value"',
    UnknownKey(:final line, :final key) => 'line $line: unknown key "$key"',
    DuplicateKey(:final line, :final key, :final first) =>
      'line $line: "$key" is already set on line $first',
    BadValue(:final line, :final key, :final problem) =>
      'line $line: $key: ${problem.message}',
    MissingKey(:final key) => 'missing key "$key"',
  };

  String get kind => switch (this) {
    SyntaxProblem() => 'syntax',
    UnknownKey() => 'unknown key',
    DuplicateKey() => 'duplicate key',
    BadValue() => 'bad value',
    MissingKey() => 'missing key',
  };
}

class SyntaxProblem extends ConfigError {
  final int line;
  const SyntaxProblem(this.line);
}

class UnknownKey extends ConfigError {
  final int line;
  final String key;
  const UnknownKey(this.line, this.key);
}

class DuplicateKey extends ConfigError {
  final int line;
  final String key;
  final int first;
  const DuplicateKey(this.line, this.key, this.first);
}

class BadValue extends ConfigError {
  final int line;
  final String key;
  final ValueProblem problem;
  const BadValue(this.line, this.key, this.problem);
}

class MissingKey extends ConfigError {
  final String key;
  const MissingKey(this.key);
}

const kinds = [
  'syntax',
  'unknown key',
  'duplicate key',
  'bad value',
  'missing key',
];

// What load returns: a config, or the list of everything wrong with the file.
sealed class LoadResult {}

class Loaded extends LoadResult {
  final Config config;
  Loaded(this.config);
}

class Failed extends LoadResult {
  final List<ConfigError> errors;
  Failed(this.errors);
}

String parseText(String text) =>
    text.isEmpty ? throw ValueProblem('must not be empty') : text;

int parseWhole(String text, int low, int high) {
  if (!RegExp(r'^[0-9]+$').hasMatch(text)) {
    throw ValueProblem('"$text" is not a whole number');
  }
  var value = 0;
  for (final unit in text.codeUnits) {
    // Stop growing past the limit, so a long number cannot overflow.
    value = (value * 10 + unit - 48).clamp(0, high + 1);
  }
  if (value < low || value > high) {
    throw ValueProblem('$text is not between $low and $high');
  }
  return value;
}

bool parseBool(String text) => switch (text) {
  'true' => true,
  'false' => false,
  _ => throw ValueProblem('"$text" is not true or false'),
};

LoadResult load(List<String> lines) {
  String? name;
  int? port;
  var debug = false;
  var timeout = 30;
  final firstLine = <String, int>{};
  final errors = <ConfigError>[];
  for (var index = 0; index < lines.length; index++) {
    final number = index + 1;
    final line = lines[index].trim();
    if (line.isEmpty || line.startsWith('#')) continue;
    final equals = line.indexOf('=');
    final key = equals == -1 ? '' : line.substring(0, equals).trim();
    if (key.isEmpty) {
      errors.add(SyntaxProblem(number));
      continue;
    }
    final text = line.substring(equals + 1).trim();
    if (!['name', 'port', 'debug', 'timeout'].contains(key)) {
      errors.add(UnknownKey(number, key));
      continue;
    }
    final first = firstLine[key];
    if (first != null) {
      errors.add(DuplicateKey(number, key, first));
      continue;
    }
    firstLine[key] = number;
    try {
      switch (key) {
        case 'name':
          name = parseText(text);
        case 'port':
          port = parseWhole(text, 1, 65535);
        case 'debug':
          debug = parseBool(text);
        default:
          timeout = parseWhole(text, 1, 3600);
      }
    } on ValueProblem catch (problem) {
      errors.add(BadValue(number, key, problem));
    }
  }
  for (final key in ['name', 'port']) {
    if (!firstLine.containsKey(key)) errors.add(MissingKey(key));
  }
  if (errors.isNotEmpty || name == null || port == null) return Failed(errors);
  return Loaded(Config(name, port, debug, timeout));
}

void main() {
  final files = <(String, List<String>)>[];
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    if (line.startsWith('== ')) {
      files.add((line.substring(3), []));
    } else if (files.isNotEmpty) {
      files.last.$2.add(line);
    }
  }
  final counts = {for (final kind in kinds) kind: 0};
  var loaded = 0;
  for (final (name, lines) in files) {
    switch (load(lines)) {
      case Loaded(:final config):
        loaded++;
        print('$name: ok');
        print('  name = ${config.name}');
        print('  port = ${config.port}');
        print('  debug = ${config.debug}');
        print('  timeout = ${config.timeout}');
      case Failed(:final errors):
        print('$name: ${errors.length} error${errors.length == 1 ? '' : 's'}');
        for (final error in errors) {
          print('  ${error.message}');
          counts[error.kind] = counts[error.kind]! + 1;
        }
    }
  }
  print('files loaded: $loaded of ${files.length}');
  print(kinds.map((kind) => '$kind: ${counts[kind]}').join(', '));
}
