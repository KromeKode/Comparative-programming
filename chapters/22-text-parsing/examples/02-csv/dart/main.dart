// Reads CSV, as RFC 4180 describes it, and prints each record as a JSON list of strings,
// one record on each line. A field in quotes can hold commas, line breaks, and "" for a
// quote. Lines can end with "\r\n" or "\n". Every record must have as many fields as the
// first. At the first error, it prints the line and column, and the exit status is 1.
import 'dart:convert';
import 'dart:io';

enum State {
  recordStart, // nothing of this record read yet
  fieldStart, // just after a comma
  unquoted, // in a field that doesn't start with a quote
  quoted, // in a field that does
  quote, // just after a quote in a quoted field: its end, or the first of ""
  cr, // just after a "\r" outside quotes, which needs a "\n" next
}

class CsvError implements Exception {
  final int line, column;
  final String message;
  CsvError(this.line, this.column, this.message);

  @override
  String toString() => 'line $line, column $column: $message';
}

String fields(int n) => n == 1 ? '1 field' : '$n fields';

/// The records in text, one at a time. Throws a CsvError at the first error.
Iterable<List<String>> records(String text) sync* {
  var state = State.recordStart;
  var record = <String>[];
  final field = StringBuffer();
  var width = 0; // how many fields the first record has, once it has ended
  // Where char is: columns count characters, from 1.
  var (line, column) = (1, 0);
  // Where the quote that began a quoted field is, or the "\r".
  var start = (0, 0);
  var recordLine = 1;
  // A String is UTF-16, and runes are its code points, so "🦊" is one character. The
  // null after them stands for the end, so that the end goes through the machine too.
  for (final char in [...text.runes.map(String.fromCharCode), null]) {
    column++;
    if (state == State.recordStart) recordLine = line;
    var ended = false; // whether char ends a record
    // Each case is a state and a character; the first that fits is taken.
    switch ((state, char)) {
      case (State.quoted, null):
        throw CsvError(start.$1, start.$2, 'quoted field is never closed');
      case (State.quoted, '"'):
        state = State.quote;
      case (State.quoted, final String c):
        field.write(c);
      case (State.quote, '"'):
        field.write('"');
        state = State.quoted;
      case (State.cr, '\n'):
        ended = true;
      case (State.cr, _):
        throw CsvError(
          start.$1,
          start.$2,
          'a carriage return without a line feed after it',
        );
      case (State.recordStart, null):
        break;
      case (State.recordStart || State.fieldStart, '"'):
        start = (line, column);
        state = State.quoted;
      case (State.unquoted, '"'):
        throw CsvError(line, column, 'a quote in an unquoted field');
      // From here on, the state is recordStart, fieldStart, unquoted, or quote.
      case (_, ','):
        record.add(field.toString());
        field.clear();
        state = State.fieldStart;
      case (_, '\n' || null):
        ended = true;
      case (_, '\r'):
        start = (line, column);
        state = State.cr;
      case (State.quote, _):
        throw CsvError(
          line,
          column,
          'expected a comma or a line end after a closing quote',
        );
      case (_, final String c):
        field.write(c);
        state = State.unquoted;
    }
    if (ended) {
      record.add(field.toString());
      field.clear();
      if (width == 0) {
        width = record.length;
      } else if (record.length != width) {
        final message =
            'record has ${fields(record.length)}, but the first has $width';
        throw CsvError(recordLine, 1, message);
      }
      yield record;
      record = [];
      state = State.recordStart;
    }
    if (char == '\n') (line, column) = (line + 1, 0);
  }
}

Future<void> main() async {
  final text = await utf8.decodeStream(stdin);
  try {
    for (final record in records(text)) {
      print(jsonEncode(record));
    }
  } on CsvError catch (err) {
    stderr.writeln('csv2json: $err');
    exitCode = 1;
  }
}
