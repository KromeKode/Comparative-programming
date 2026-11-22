import 'dart:io';

// Dart has no macros, and compiled Dart programs have no reflection, so a
// book is a map from field names to values. The fields are written once,
// here, with their defaults, and the type of each default says how to parse
// its column. A map keeps its keys in the order they were added.
const defaultBook = <String, Object>{
  'title': '',
  'author': '',
  'year': 0,
  'pages': 0,
  'in_print': false,
};

class BadValue implements Exception {
  final String message;
  BadValue(this.message);
}

final whole = RegExp(r'^-?[0-9]+$');

// A switch on the default's type picks the parser.
Object parse(String name, String text) => switch (defaultBook[name]) {
  String _ => text,
  int _ when whole.hasMatch(text) => int.parse(text),
  int _ => throw BadValue('$name: "$text" is not a whole number'),
  bool _ when text == 'true' || text == 'false' => text == 'true',
  bool _ => throw BadValue('$name: "$text" is not true or false'),
  _ => throw StateError('no parser for $name'),
};

String show(Object value) => value is String ? '"$value"' : '$value';

String describe(Map<String, Object> book) {
  final parts = book.entries.map(
    (entry) => '${entry.key}=${show(entry.value)}',
  );
  return 'Book(${parts.join(', ')})';
}

Map<String, Object> readBook(List<String> columns, List<String> values) {
  if (values.length != columns.length) {
    throw BadValue('expected ${columns.length} values, got ${values.length}');
  }
  final book = Map.of(defaultBook);
  for (final (i, name) in columns.indexed) {
    book[name] = parse(name, values[i]);
  }
  return book;
}

List<String> split(String line) => [
  for (final part in line.split(',')) part.trim(),
];

String plural(int count, String word) => '$count $word${count == 1 ? '' : 's'}';

void main() {
  final lines = <String>[];
  for (String? line; (line = stdin.readLineSync()) != null;) {
    if (line!.trim().isNotEmpty) lines.add(line);
  }
  if (lines.isEmpty) return;
  final columns = split(lines.first);
  for (final (i, name) in columns.indexed) {
    if (!defaultBook.containsKey(name)) {
      print('unknown column: $name');
      return;
    }
    if (columns.take(i).contains(name)) {
      print('repeated column: $name');
      return;
    }
  }

  var books = 0, bad = 0;
  for (final (i, line) in lines.skip(1).indexed) {
    try {
      print(describe(readBook(columns, split(line))));
      books++;
    } on BadValue catch (error) {
      print('row ${i + 1}: ${error.message}');
      bad++;
    }
  }
  print('${plural(books, 'book')}, ${plural(bad, 'bad row')}');
}
