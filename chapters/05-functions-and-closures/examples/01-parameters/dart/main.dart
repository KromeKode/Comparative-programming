// Parameters in {} are named, and optional unless marked required.
String greet(
  String name, {
  String greeting = 'Hello',
  String punctuation = '!',
}) => '$greeting, $name$punctuation';

// Dart has no variadic functions: take a List.
int total(List<int> values) {
  var sum = 0;
  for (final value in values) {
    sum += value;
  }
  return sum;
}

void main() {
  print(greet('Ada'));
  print(greet('Grace', greeting: 'Hi'));
  print(greet('Linus', punctuation: '?'));

  print('total() = ${total([])}');
  print('total(1, 2, 3) = ${total([1, 2, 3])}');
  final numbers = [4, 3, 2, 1];
  print('total of [4, 3, 2, 1] = ${total(numbers)}');
}
