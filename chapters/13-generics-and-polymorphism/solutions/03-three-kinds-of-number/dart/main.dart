import 'dart:io';

class CalcError implements Exception {
  final String message;
  CalcError(this.message);

  @override
  String toString() => message;
}

// What evaluate needs from a number type. N extends Arithmetic<N> makes every
// operator take and return the same type. / throws CalcError when dividing
// by zero.
abstract interface class Arithmetic<N extends Arithmetic<N>> {
  N operator +(N other);
  N operator -(N other);
  N operator *(N other);
  N operator /(N other);
}

final whole = RegExp(r'^-?[0-9]+$');

int? parseWhole(String text) =>
    whole.hasMatch(text) ? int.tryParse(text) : null;

class Int implements Arithmetic<Int> {
  final int value;
  Int(this.value);

  static Int? parse(String text) {
    final n = parseWhole(text);
    return n == null ? null : Int(n);
  }

  @override
  Int operator +(Int other) => Int(value + other.value);
  @override
  Int operator -(Int other) => Int(value - other.value);
  @override
  Int operator *(Int other) => Int(value * other.value);

  // ~/ divides and rounds toward zero.
  @override
  Int operator /(Int other) {
    if (other.value == 0) throw CalcError('division by zero');
    return Int(value ~/ other.value);
  }

  @override
  String toString() => '$value';
}

// Always in lowest terms, with a positive bottom.
class Frac implements Arithmetic<Frac> {
  final int top, bottom;
  Frac._(this.top, this.bottom);

  factory Frac(int top, int bottom) {
    final sign = bottom < 0 ? -1 : 1;
    final divisor = top.gcd(bottom); // never negative; 0.gcd(b) is b
    return Frac._(sign * top ~/ divisor, sign * bottom ~/ divisor);
  }

  static Frac? parse(String text) {
    final match = RegExp(r'^(-?[0-9]+)(?:/([0-9]+))?$').firstMatch(text);
    if (match == null) return null;
    final bottom = int.parse(match[2] ?? '1');
    // "1/0" is not a fraction: the bottom must be above zero.
    return bottom > 0 ? Frac(int.parse(match[1]!), bottom) : null;
  }

  @override
  Frac operator +(Frac o) =>
      Frac(top * o.bottom + o.top * bottom, bottom * o.bottom);
  @override
  Frac operator -(Frac o) =>
      Frac(top * o.bottom - o.top * bottom, bottom * o.bottom);
  @override
  Frac operator *(Frac o) => Frac(top * o.top, bottom * o.bottom);

  @override
  Frac operator /(Frac o) {
    if (o.top == 0) throw CalcError('division by zero');
    return Frac(top * o.bottom, bottom * o.top);
  }

  @override
  String toString() => bottom == 1 ? '$top' : '$top/$bottom';
}

// Always 0 to 6. Dart's % never gives a negative result for a positive
// divisor, so -1 % 7 is already 6.
class Mod7 implements Arithmetic<Mod7> {
  final int value;
  Mod7(int n) : value = n % 7;

  static Mod7? parse(String text) {
    final n = parseWhole(text);
    return n == null ? null : Mod7(n);
  }

  @override
  Mod7 operator +(Mod7 other) => Mod7(value + other.value);
  @override
  Mod7 operator -(Mod7 other) => Mod7(value - other.value);
  @override
  Mod7 operator *(Mod7 other) => Mod7(value * other.value);

  // x to the power 5 is the inverse of x when 7 is prime: x * x^5 = x^6 = 1.
  @override
  Mod7 operator /(Mod7 other) {
    if (other.value == 0) throw CalcError('division by zero');
    return this * other * other * other * other * other;
  }

  @override
  String toString() => '$value';
}

// Run the tokens on a stack of N. Dart can't call a static method through a
// type parameter, so the caller passes in parse: Int.parse, Frac.parse, or
// Mod7.parse.
N evaluate<N extends Arithmetic<N>>(
  List<String> tokens,
  N? Function(String) parse,
) {
  final stack = <N>[];
  for (final token in tokens) {
    if (const {'+', '-', '*', '/'}.contains(token)) {
      if (stack.length < 2) throw CalcError('not enough numbers');
      final right = stack.removeLast();
      final left = stack.removeLast();
      stack.add(switch (token) {
        '+' => left + right,
        '-' => left - right,
        '*' => left * right,
        _ => left / right,
      });
    } else {
      stack.add(parse(token) ?? (throw CalcError('bad token "$token"')));
    }
  }
  if (stack.length != 1)
    throw CalcError('expected one result, found ${stack.length}');
  return stack.single;
}

void main() {
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    final words = line.trim().split(RegExp(r'\s+'));
    if (words.first.isEmpty) continue;
    final [kind, ...tokens] = words;
    try {
      final result = switch (kind) {
        'int' => evaluate(tokens, Int.parse),
        'frac' => evaluate(tokens, Frac.parse),
        'mod7' => evaluate(tokens, Mod7.parse),
        _ => throw CalcError('unknown kind "$kind"'),
      };
      print(result);
    } on CalcError catch (error) {
      print('error: $error');
    }
  }
}
