// Reads one JSON text on stdin, strictly as RFC 8259 has it, and prints it compactly:
// no spaces, keys in the order they came, numbers as they were written, and strings
// written again with only the escapes they need. Or prints the first error's line and
// column, and exits with 1.
import 'dart:convert';
import 'dart:io';

const maxDepth = 1000;

/// s as a JSON string, with only the escapes it needs. jsonEncode(s) writes the same, for
/// a string with no lone surrogates; this shows what that is.
String quote(String s) {
  final out = StringBuffer('"');
  for (final c in s.runes) {
    out.write(switch (c) {
      0x22 => r'\"',
      0x5C => r'\\',
      0x08 => r'\b',
      0x0C => r'\f',
      0x0A => r'\n',
      0x0D => r'\r',
      0x09 => r'\t',
      < 0x20 => '\\u${c.toRadixString(16).padLeft(4, '0')}',
      _ => String.fromCharCode(c),
    });
  }
  out.write('"');
  return out.toString();
}

class JsonError implements Exception {
  final int at; // an index into the code points
  final String message;
  JsonError(this.at, this.message);
}

const end = -1; // stands for the end of the text

class Parser {
  // A String is UTF-16, so these are its code points, and an index counts them, as
  // columns do.
  final List<int> text;
  var _i = 0;
  var _depth = 0;

  Parser(String text) : text = text.runes.toList();

  int _peek() => _i < text.length ? text[_i] : end;

  bool _is(String c) => _peek() == c.codeUnitAt(0);

  JsonError _fail(String expected) {
    final c = _peek();
    final found = c == end
        ? 'the end of the input'
        : quote(String.fromCharCode(c));
    return JsonError(_i, 'expected $expected, found $found');
  }

  void _space() {
    while (_is(' ') || _is('\t') || _is('\n') || _is('\r')) {
      _i++;
    }
  }

  bool _startsWith(int at, String word) =>
      at + word.length <= text.length &&
      String.fromCharCodes(text, at, at + word.length) == word;

  String _value() {
    if (_is('{') || _is('[')) {
      _depth++;
      if (_depth > maxDepth)
        throw JsonError(_i, 'nesting deeper than $maxDepth');
      final out = _is('{') ? _object() : _array();
      _depth--;
      return out;
    }
    if (_is('"')) return quote(_string());
    if (_is('-') || _isDigit()) return _number();
    for (final word in ['true', 'false', 'null']) {
      if (_startsWith(_i, word)) {
        _i += word.length;
        return word;
      }
    }
    throw _fail('a value');
  }

  String _object() {
    _i++; // the {
    _space();
    if (_is('}')) {
      _i++;
      return '{}';
    }
    final members = <String>[];
    final keys = <String>{};
    while (true) {
      if (!_is('"')) {
        throw _fail(members.isEmpty ? 'a string key or "}"' : 'a string key');
      }
      final at = _i;
      final key = _string();
      // add returns false if the set had it already.
      if (!keys.add(key)) throw JsonError(at, 'duplicate key ${quote(key)}');
      _space();
      if (!_is(':')) throw _fail('":"');
      _i++;
      _space();
      members.add('${quote(key)}:${_value()}');
      _space();
      if (_is('}')) {
        _i++;
        return '{${members.join(',')}}';
      }
      if (!_is(',')) throw _fail('"," or "}"');
      _i++;
      _space();
    }
  }

  String _array() {
    _i++; // the [
    _space();
    if (_is(']')) {
      _i++;
      return '[]';
    }
    final items = <String>[];
    while (true) {
      items.add(_value());
      _space();
      if (_is(']')) {
        _i++;
        return '[${items.join(',')}]';
      }
      if (!_is(',')) throw _fail('"," or "]"');
      _i++;
      _space();
    }
  }

  bool _isDigit() => _peek() >= 0x30 && _peek() <= 0x39;

  void _digits() {
    if (!_isDigit()) throw _fail('a digit');
    while (_isDigit()) {
      _i++;
    }
  }

  String _number() {
    final start = _i;
    if (_is('-')) _i++;
    if (_is('0')) {
      _i++; // a 0 can't have digits after it
    } else {
      _digits();
    }
    if (_is('.')) {
      _i++;
      _digits();
    }
    if (_is('e') || _is('E')) {
      _i++;
      if (_is('+') || _is('-')) _i++;
      _digits();
    }
    return String.fromCharCodes(text, start, _i); // as it was written
  }

  /// The four hex digits at text[at], or null if they aren't.
  int? _hex4(int at) {
    if (at + 4 > text.length) return null;
    final digits = String.fromCharCodes(text, at, at + 4);
    return RegExp(r'^[0-9a-fA-F]{4}$').hasMatch(digits)
        ? int.parse(digits, radix: 16)
        : null;
  }

  static const _simple = {
    '"': '"', r'\': r'\', '/': '/', //
    'b': '\b', 'f': '\f', 'n': '\n', 'r': '\r', 't': '\t',
  };

  /// The string at i, with its escapes undone.
  String _string() {
    final start = _i;
    _i++; // the "
    final out = StringBuffer();
    while (true) {
      final c = _peek();
      if (c == end) throw JsonError(start, 'string is never closed');
      if (_is('"')) {
        _i++;
        return out.toString();
      }
      if (c < 0x20) throw JsonError(_i, 'control character in a string');
      if (!_is(r'\')) {
        out.writeCharCode(c);
        _i++;
        continue;
      }
      if (_i + 1 == text.length)
        throw JsonError(start, 'string is never closed');
      final escape = String.fromCharCode(text[_i + 1]);
      final simple = _simple[escape];
      if (simple != null) {
        out.write(simple);
        _i += 2;
        continue;
      }
      var code = escape == 'u' ? _hex4(_i + 2) : null;
      if (code == null) throw JsonError(_i, 'invalid escape');
      final at = _i;
      _i += 6;
      if (code >= 0xDC00 && code <= 0xDFFF) {
        throw JsonError(at, 'unpaired surrogate');
      }
      if (code >= 0xD800 && code <= 0xDBFF) {
        // A character above U+FFFF is two escapes, a high surrogate and a low one.
        final low = _startsWith(_i, r'\u') ? _hex4(_i + 2) : null;
        if (low == null || low < 0xDC00 || low > 0xDFFF) {
          throw JsonError(at, 'unpaired surrogate');
        }
        code = 0x10000 + ((code - 0xD800) << 10) + (low - 0xDC00);
        _i += 6;
      }
      out.writeCharCode(code);
    }
  }

  String parse() {
    _space();
    final out = _value();
    _space();
    if (_i < text.length) throw _fail('the end of the input');
    return out;
  }
}

Future<void> main() async {
  final bytes = await stdin.expand((chunk) => chunk).toList();
  // Dart's UTF-8 decoder drops a byte order mark at the start, which JSON doesn't allow,
  // so this puts it back, for the parser to find.
  final bom =
      bytes.length >= 3 &&
      bytes[0] == 0xEF &&
      bytes[1] == 0xBB &&
      bytes[2] == 0xBF;
  final parser = Parser((bom ? '\u{FEFF}' : '') + utf8.decode(bytes));
  try {
    print(parser.parse());
  } on JsonError catch (err) {
    // Lines end at "\n", and columns count characters from 1.
    final before = parser.text.sublist(0, err.at);
    final line = before.where((c) => c == 0x0A).length + 1;
    final column = err.at - before.lastIndexOf(0x0A);
    stderr.writeln('jsoncheck: line $line, column $column: ${err.message}');
    exitCode = 1;
  }
}
