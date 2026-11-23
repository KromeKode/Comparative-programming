// Reads arithmetic, one expression on each line, such as 1 + 2 * 3, and prints each with
// its parentheses written out, and its value: (1 + (2 * 3)) = 7. The numbers are whole
// and fit in 64 bits; the operators are + - * / % ^ and unary -, with ( and ). / and %
// round toward zero, as in C. An error is printed with its line and a caret under its
// column, the next line is read, and at the end, the exit status is 1.
import 'dart:convert';
import 'dart:io';

class CalcError implements Exception {
  final int column; // counted in characters, from 1
  final String message;
  CalcError(this.column, this.message);
}

// The lexer turns the text into tokens: numbers, and operators and parentheses, each
// with its column.
enum Kind { number, operator, end } // operator includes the parentheses

class Token {
  final Kind kind;
  final String text;
  final int value; // a number's value
  final int column;
  Token(this.kind, this.text, this.column, [this.value = 0]);

  String describe() => kind == Kind.end ? 'the end of the line' : '"$text"';
}

bool isDigit(String c) => c.compareTo('0') >= 0 && c.compareTo('9') <= 0;

List<Token> tokens(String line) {
  // A String is UTF-16, so this splits it into code points, and i + 1 is the column.
  final chars = line.runes.map(String.fromCharCode).toList();
  final found = <Token>[];
  var i = 0;
  while (i < chars.length) {
    final c = chars[i];
    if (c == ' ' || c == '\t') {
      i++;
    } else if (isDigit(c)) {
      final start = i;
      while (i < chars.length && isDigit(chars[i])) {
        i++;
      }
      final text = chars.sublist(start, i).join();
      // tryParse gives null if the number doesn't fit, where parse would throw.
      final value = int.tryParse(text);
      if (value == null) {
        throw CalcError(start + 1, 'number too large for 64 bits');
      }
      found.add(Token(Kind.number, text, start + 1, value));
    } else if ('+-*/%^()'.contains(c)) {
      found.add(Token(Kind.operator, c, i + 1));
      i++;
    } else {
      throw CalcError(i + 1, 'unexpected "$c"');
    }
  }
  found.add(Token(Kind.end, '', chars.length + 1));
  return found;
}

// The parser turns the tokens into a tree, whose nodes are of three kinds. A sealed
// class can have subclasses only in this file, so a switch on an Expr can know that it
// has a case for each. column is where the operator is, for an error such as overflow.
sealed class Expr {}

class Number extends Expr {
  final int value;
  Number(this.value);
}

class Negate extends Expr {
  final Expr operand;
  final int column;
  Negate(this.operand, this.column);
}

class Binary extends Expr {
  final String op;
  final Expr left, right;
  final int column;
  Binary(this.op, this.left, this.right, this.column);
}

// How tightly each operator holds the expressions on its left and right: the higher,
// the tighter. Left is less than right for left-associative operators, so 1 - 2 - 3 is
// (1 - 2) - 3, and more for ^, so 2 ^ 3 ^ 2 is 2 ^ (3 ^ 2).
const infix = {
  '+': (1, 2),
  '-': (1, 2),
  '*': (3, 4),
  '/': (3, 4),
  '%': (3, 4),
  '^': (8, 7),
};
// Unary minus holds less tightly than ^, so -2 ^ 2 is -(2 ^ 2), and more than *.
const prefix = 5;

class Parser {
  final List<Token> _tokens;
  var _at = 0;

  Parser(String line) : _tokens = tokens(line);

  Token _next() {
    final token = _tokens[_at];
    if (token.kind != Kind.end) _at++;
    return token;
  }

  /// Parses the longest expression whose operators hold at least minPower.
  Expr _expression(int minPower) {
    final token = _next();
    var left = switch (token) {
      Token(kind: Kind.number, :final value) => Number(value),
      Token(text: '-') => Negate(_expression(prefix), token.column),
      Token(text: '(') => _parenthesized(),
      _ => throw CalcError(
        token.column,
        'expected a number, "(", or "-", found ${token.describe()}',
      ),
    };
    while (true) {
      final token = _tokens[_at];
      final powers = token.kind == Kind.operator ? infix[token.text] : null;
      if (powers == null || powers.$1 < minPower) return left;
      _next();
      left = Binary(token.text, left, _expression(powers.$2), token.column);
    }
  }

  Expr _parenthesized() {
    final inside = _expression(0);
    final close = _next();
    if (close.text != ')') {
      throw CalcError(close.column, 'expected ")", found ${close.describe()}');
    }
    return inside;
  }

  Expr parse() {
    final expr = _expression(0);
    final token = _next();
    if (token.kind != Kind.end) {
      throw CalcError(
        token.column,
        'expected an operator, found ${token.describe()}',
      );
    }
    return expr;
  }
}

String show(Expr expr) => switch (expr) {
  Number(:final value) => '$value',
  Negate(:final operand) => '(-${show(operand)})',
  Binary(:final op, :final left, :final right) =>
    '(${show(left)} $op ${show(right)})',
};

// Dart's ints are 64 bits, and wrap around when they overflow, with no error. So the
// arithmetic is done with BigInts, which have no limit, and each result is checked.
int fit(BigInt value, int column) {
  if (!value.isValidInt) throw CalcError(column, 'overflow');
  return value.toInt();
}

// Squares the base for each bit of the exponent, and stops at the first overflow.
int power(int base, int exponent, int column) {
  var result = 1;
  while (exponent > 0) {
    if (exponent.isOdd) {
      result = fit(BigInt.from(result) * BigInt.from(base), column);
    }
    exponent ~/= 2;
    if (exponent > 0) base = fit(BigInt.from(base) * BigInt.from(base), column);
  }
  return result;
}

int evaluate(Expr expr) {
  switch (expr) {
    case Number(:final value):
      return value;
    case Negate(:final operand, :final column):
      return fit(-BigInt.from(evaluate(operand)), column);
    case Binary(:final op, :final left, :final right, :final column):
      final (a, b) = (
        BigInt.from(evaluate(left)),
        BigInt.from(evaluate(right)),
      );
      if ((op == '/' || op == '%') && b == BigInt.zero) {
        throw CalcError(column, 'division by zero');
      }
      return switch (op) {
        '+' => fit(a + b, column),
        '-' => fit(a - b, column),
        '*' => fit(a * b, column),
        // ~/ rounds toward zero. % would give a result that is never negative, so
        // -7 % 2 would be 1; remainder has the sign of a, as C's % does.
        '/' => fit(a ~/ b, column),
        '%' => fit(a.remainder(b), column),
        _ when b.isNegative => throw CalcError(column, 'negative exponent'),
        _ => power(a.toInt(), b.toInt(), column),
      };
  }
}

Future<void> main() async {
  var status = 0;
  final lines = (await utf8.decodeStream(stdin)).split('\n');
  // The last is empty when the input ended with a newline, or was empty.
  if (lines.last == '') lines.removeLast();
  for (final (index, text) in lines.indexed) {
    final line = text.endsWith('\r')
        ? text.substring(0, text.length - 1)
        : text;
    if (line.replaceAll(RegExp('[ \t]'), '').isEmpty) continue;
    try {
      final expr = Parser(line).parse();
      print('${show(expr)} = ${evaluate(expr)}');
    } on CalcError catch (err) {
      // The caret goes under the column. A tab before it stays a tab, so that the
      // caret lines up with the text whatever a tab's width is.
      final pad = line.runes
          .take(err.column - 1)
          .map((c) => String.fromCharCode(c) == '\t' ? '\t' : ' ')
          .join();
      stderr.writeln(
        'calc: line ${index + 1}, column ${err.column}: ${err.message}',
      );
      stderr.writeln('  $line\n  $pad^');
      status = 1;
    }
  }
  exitCode = status;
}
