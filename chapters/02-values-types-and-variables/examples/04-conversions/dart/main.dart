void main() {
  final parsed = int.parse('42'); // throws FormatException on bad input
  print('"42" + 1 as a number: ${parsed + 1}');

  const x = 3.99;
  print('3.99 truncated: ${x.toInt()}');
  print('-3.99 truncated: ${(-x).toInt()}');
  print('2.5 rounded: ${2.5.round()}'); // halfway rounds away from zero
  print('-2.5 rounded: ${(-2.5).round()}');

  print('7 / 2 as integers: ${7 ~/ 2}'); // ~/ is integer division
  print('7 / 2 as floats: ${7 / 2}'); // / always gives a double
}
