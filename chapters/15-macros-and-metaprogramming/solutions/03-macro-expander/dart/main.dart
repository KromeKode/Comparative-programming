import 'dart:io';

// params is null for a macro that takes no parentheses.
typedef Macro = ({List<String>? params, String body});

class ExpandError implements Exception {
  final String message;
  ExpandError(this.message);
}

final macros = <String, Macro>{};

final wordChar = RegExp(r'\w');
final namePattern = RegExp(r'^[A-Za-z_]\w*$');

bool isWordChar(String c) => wordChar.hasMatch(c);

bool isName(String word) => namePattern.hasMatch(word);

int wordEnd(String text, int start) {
  var end = start;
  while (end < text.length && isWordChar(text[end])) {
    end++;
  }
  return end;
}

int spacesEnd(String text, int start) {
  var end = start;
  while (end < text.length && text[end] == ' ') {
    end++;
  }
  return end;
}

// The index just past the string that starts at text[start], a quote.
int stringEnd(String text, int start) {
  final close = text.indexOf('"', start + 1);
  return close < 0 ? text.length : close + 1;
}

// Splits the arguments of a call at its top-level commas, and trims them.
// Returns them with the index just past the ")".
(List<String>, int) splitArgs(String text, int open, String name) {
  final args = <String>[];
  var depth = 0;
  var start = open + 1, i = open + 1;
  while (i < text.length) {
    switch (text[i]) {
      case '"':
        i = stringEnd(text, i);
        continue;
      case '(':
        depth++;
      case ')' when depth > 0:
        depth--;
      case ')':
        args.add(text.substring(start, i).trim());
        return (args, i + 1);
      case ',' when depth == 0:
        args.add(text.substring(start, i).trim());
        start = i + 1;
    }
    i++;
  }
  throw ExpandError('unterminated call to $name');
}

// The macro's body, with each parameter replaced by its argument.
String substitute(Macro macro, List<String> args, Set<String> disabled) {
  final raw = {for (final (p, param) in macro.params!.indexed) param: args[p]};
  final expanded = <String, String>{}; // an argument is expanded only if used
  final body = macro.body;
  final out = StringBuffer();
  var i = 0;
  while (i < body.length) {
    var end = i + 1;
    if (body[i] == '"') {
      end = stringEnd(body, i);
    } else if (body[i] == '#') {
      // # and a parameter make a string of the argument as written.
      final start = spacesEnd(body, i + 1);
      final stop = wordEnd(body, start);
      final arg = raw[body.substring(start, stop)];
      if (arg != null) {
        out.write('"$arg"');
        i = stop;
        continue;
      }
    } else if (isWordChar(body[i])) {
      end = wordEnd(body, i);
      final word = body.substring(i, end);
      final arg = raw[word];
      if (arg != null) {
        out.write(expanded.putIfAbsent(word, () => expand(arg, disabled)));
        i = end;
        continue;
      }
    }
    out.write(body.substring(i, end));
    i = end;
  }
  return out.toString();
}

// Expands every macro in text, except those in disabled.
String expand(String text, Set<String> disabled) {
  final out = StringBuffer();
  var i = 0;
  while (i < text.length) {
    var end = i + 1;
    if (text[i] == '"') {
      end = stringEnd(text, i);
    } else if (isWordChar(text[i])) {
      end = wordEnd(text, i);
      final word = text.substring(i, end);
      final macro = macros[word];
      if (macro != null && !disabled.contains(word)) {
        // Inside its own expansion, a macro's name is left alone.
        final inner = {...disabled, word};
        final params = macro.params;
        if (params == null) {
          out.write(expand(macro.body, inner));
          i = end;
          continue;
        }
        final paren = spacesEnd(text, end);
        if (paren < text.length && text[paren] == '(') {
          var (args, after) = splitArgs(text, paren, word);
          if (params.isEmpty && args.length == 1 && args[0] == '') args = [];
          final count = params.length;
          if (args.length != count) {
            final noun = count == 1 ? 'argument' : 'arguments';
            throw ExpandError('$word takes $count $noun, got ${args.length}');
          }
          out.write(expand(substitute(macro, args, disabled), inner));
          i = after;
          continue;
        }
      }
    }
    out.write(text.substring(i, end));
    i = end;
  }
  return out.toString();
}

void define(String rest) {
  final end = wordEnd(rest, 0);
  final name = rest.substring(0, end);
  if (!isName(name)) throw ExpandError('bad #define');
  if (end == rest.length || rest[end] != '(') {
    macros[name] = (params: null, body: rest.substring(end).trim());
    return;
  }
  final close = rest.indexOf(')', end);
  if (close < 0) throw ExpandError('bad #define');
  final inside = rest.substring(end + 1, close);
  final params = inside.trim().isEmpty
      ? <String>[]
      : [for (final param in inside.split(',')) param.trim()];
  if (!params.every(isName) || params.toSet().length != params.length) {
    throw ExpandError('bad #define');
  }
  macros[name] = (params: params, body: rest.substring(close + 1).trim());
}

// Returns the line to print, or null for a directive.
String? run(String line) {
  final whole = line.trim();
  final space = whole.indexOf(' ');
  final directive = space < 0 ? whole : whole.substring(0, space);
  final rest = space < 0 ? '' : whole.substring(space + 1).trim();
  switch (directive) {
    case '#define':
      define(rest);
      return null;
    case '#undef':
      if (!isName(rest)) throw ExpandError('bad #undef');
      macros.remove(rest);
      return null;
  }
  return expand(line, {});
}

void main() {
  for (String? line; (line = stdin.readLineSync()) != null;) {
    try {
      final result = run(line!);
      if (result != null) print(result);
    } on ExpandError catch (error) {
      print('error: ${error.message}');
    }
  }
}
