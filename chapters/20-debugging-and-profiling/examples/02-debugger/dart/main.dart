// Evaluates arithmetic with +, -, *, and parentheses: each argument, or each line of
// input if there are none. Section 20.5 runs it in a debugger, to watch the parser
// call itself. It expects well-formed expressions, and doesn't check.
import 'dart:io';

/// A recursive descent parser, with one method for each rule of the grammar.
class Parser {
  Parser(this.text);

  final String text;
  var pos = 0;

  /// Skips spaces, and gives the next character, or '' at the end.
  String peek() {
    while (pos < text.length && text[pos] == ' ') {
      pos++;
    }
    return pos < text.length ? text[pos] : '';
  }

  // expr = term { ("+" | "-") term }
  int expr() {
    var value = term();
    while (peek() == '+' || peek() == '-') {
      final op = text[pos++];
      final right = term();
      value = op == '+' ? value + right : value - right;
    }
    return value;
  }

  // term = factor { "*" factor }
  int term() {
    var value = factor();
    while (peek() == '*') {
      pos++;
      value *= factor();
    }
    return value;
  }

  // factor = number | "(" expr ")"
  int factor() {
    if (peek() == '(') {
      pos++;
      final value = expr();
      peek();
      pos++; // the ")"
      return value;
    }
    final start = pos;
    while (pos < text.length && '0123456789'.contains(text[pos])) {
      pos++;
    }
    return int.parse(text.substring(start, pos));
  }
}

int evaluate(String text) => Parser(text).expr();

void main(List<String> args) {
  for (final text in args) {
    print(evaluate(text));
  }
  if (args.isNotEmpty) return;
  for (String? line; (line = stdin.readLineSync()) != null;) {
    print(evaluate(line!));
  }
}
