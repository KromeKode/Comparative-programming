void main() {
  // ~/ is integer division: it truncates toward zero, like C.
  print('7 ~/ 2 = ${7 ~/ 2}');
  print('-7 ~/ 2 = ${-7 ~/ 2}');
  // % is Euclidean modulo: for a nonzero divisor the result is never negative.
  print('7 % 3 = ${7 % 3}');
  print('-7 % 3 = ${-7 % 3}');
  print('7 % -3 = ${7 % -3}');
  // / always returns a double, even for two ints.
  print('7 / 2 = ${7 / 2}');
  // remainder() truncates, which gives C's answer.
  print('(-7).remainder(3) = ${(-7).remainder(3)}');
}
