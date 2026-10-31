// A fraction kept in lowest terms, with the sign on the numerator.
class Fraction implements Comparable<Fraction> {
  // A leading _ makes a name private to this file (its library).
  final int _numerator;
  final int _denominator; // always positive

  // A factory constructor can do work before it makes the object.
  factory Fraction(int numerator, int denominator) {
    if (denominator == 0) {
      throw ArgumentError('denominator is zero');
    }
    final sign = denominator.sign;
    final divisor = numerator.gcd(denominator);
    return Fraction._(
      sign * numerator ~/ divisor,
      sign * denominator ~/ divisor,
    );
  }

  // The real constructor is private, so every fraction goes through the
  // factory above.
  Fraction._(this._numerator, this._denominator);

  Fraction operator +(Fraction other) => Fraction(
    _numerator * other._denominator + other._numerator * _denominator,
    _denominator * other._denominator,
  );

  // == on objects compares identity unless a class overrides it, and a class
  // that overrides == must override hashCode to match.
  @override
  bool operator ==(Object other) =>
      other is Fraction &&
      _numerator == other._numerator &&
      _denominator == other._denominator;

  @override
  int get hashCode => Object.hash(_numerator, _denominator);

  // compareTo is what List.sort uses; < is defined from it.
  @override
  int compareTo(Fraction other) =>
      _numerator * other._denominator - other._numerator * _denominator;

  bool operator <(Fraction other) => compareTo(other) < 0;

  @override
  String toString() =>
      _denominator == 1 ? '$_numerator' : '$_numerator/$_denominator';
}

String yesNo(bool value) => value ? 'yes' : 'no';

void main() {
  final half = Fraction(1, 2), third = Fraction(1, 3);
  print('1/2 + 1/3 = ${half + third}');
  print('2/-4 is stored as ${Fraction(2, -4)}');
  print('6/3 is stored as ${Fraction(6, 3)}');
  try {
    Fraction(1, 0);
  } on ArgumentError catch (error) {
    // Only to show the check: real code would not pass 0 and catch the Error.
    print('1/0: error: ${error.message}');
  }
  print('1/2 == 2/4: ${yesNo(half == Fraction(2, 4))}');
  print('1/3 < 1/2: ${yesNo(third < half)}');
  final fractions = [Fraction(3, 4), third, Fraction(-1, 2), Fraction(2, 4)];
  fractions.sort();
  print('sorted: ${fractions.join(', ')}');
}
