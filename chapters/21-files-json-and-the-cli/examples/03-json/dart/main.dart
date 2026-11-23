// Reads a JSON file of books and prints a report on them, as JSON: how many there are,
// the oldest, the books with each tag, and the books with none. jsonDecode gives maps
// and lists, and a pattern checks that each book has the fields it needs.
import 'dart:collection';
import 'dart:convert';
import 'dart:io';

class Book {
  final String title, author;
  final int year;
  final List<String> tags;

  Book(this.title, this.author, this.year, this.tags);
}

/// Returns the book, or throws a FormatException if json isn't one.
Book bookFromJson(Object? json) {
  // A map pattern matches a map with these keys, and any others; String title matches
  // only a String, and names it title.
  if (json
      case final Map<String, Object?> map &&
          {'title': String title, 'author': String author, 'year': int year}) {
    final tags = map['tags'] ?? const <Object?>[]; // missing, or null: no tags
    if (tags is! List || tags.any((tag) => tag is! String)) {
      throw FormatException('$title: tags must be a list of strings');
    }
    return Book(title, author, year, List<String>.from(tags));
  }
  throw const FormatException('each book needs a title, an author, and a year');
}

List<Book> readBooks(String text) {
  final data = jsonDecode(text); // throws a FormatException if it isn't JSON
  if (data case {'books': List<Object?> books}) {
    return books.map(bookFromJson).toList();
  }
  throw const FormatException('expected an object with a list of books');
}

Map<String, Object?> report(List<Book> books) {
  Book? oldest; // the first of the oldest, when two have the same year
  for (final book in books) {
    if (oldest == null || book.year < oldest.year) oldest = book;
  }
  // A SplayTreeMap keeps its keys in order; a plain map, in the order they went in.
  final tags = SplayTreeMap<String, List<String>>();
  for (final book in books) {
    for (final tag in book.tags) {
      tags.putIfAbsent(tag, () => []).add(book.title);
    }
  }
  return {
    'count': books.length,
    'oldest': oldest == null ? null : '${oldest.title}, by ${oldest.author}',
    'tags': tags,
    'untagged': [
      for (final book in books)
        if (book.tags.isEmpty) book.title,
    ],
  };
}

void main(List<String> arguments) {
  if (arguments.length != 1) {
    stderr.writeln('usage: books FILE');
    exit(2);
  }
  final path = arguments[0];
  final List<Book> books;
  try {
    books = readBooks(File(path).readAsStringSync());
  } on FileSystemException catch (error) {
    stderr.writeln('books: $path: ${error.osError?.message ?? error.message}');
    exit(1);
  } on FormatException catch (error) {
    stderr.writeln('books: $path: ${error.message}');
    exit(1);
  }
  print(const JsonEncoder.withIndent('  ').convert(report(books)));
}
