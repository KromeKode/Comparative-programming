// A generic class. T is a type parameter, and unlike in Java, Kotlin, or
// TypeScript, a Stack<int> still knows its T at run time.
class Stack<T> {
  final _items = <T>[];

  void push(T item) => _items.add(item);

  T? pop() => _items.isEmpty ? null : _items.removeLast();

  T? peek() => _items.isEmpty ? null : _items.last;

  int get length => _items.length;
}

// A generic function. int is Comparable<num>, not Comparable<int>, so the
// bound is Comparable<Object>, which both int and String meet.
T? largest<T extends Comparable<Object>>(List<T> items) {
  if (items.isEmpty) return null;
  var best = items.first;
  for (final item in items) {
    if (item.compareTo(best) > 0) best = item;
  }
  return best;
}

// Two type parameters. swap returns a Pair with them the other way round.
class Pair<A, B> {
  final A first;
  final B second;

  const Pair(this.first, this.second);

  Pair<B, A> swap() => Pair(second, first);

  @override
  String toString() => '($first, $second)';
}

String show<T>(T? value) => value?.toString() ?? 'none';

void report<T>(String name, Stack<T> stack) {
  final popped = show(stack.pop());
  final top = show(stack.peek());
  print('$name: popped $popped, top is $top, ${stack.length} left');
}

void main() {
  final numbers = Stack<int>();
  numbers.push(3);
  numbers.push(1);
  numbers.push(4);
  report('numbers', numbers);

  final words = Stack<String>();
  words.push('pear');
  words.push('fig');
  report('words', words);

  final empty = Stack<double>();
  report('empty', empty);

  print('largest number: ${show(largest([3, 1, 4, 1, 5]))}');
  print('largest word: ${show(largest(['pear', 'fig', 'apple']))}');
  print('largest of nothing: ${show(largest(<int>[]))}');

  final pair = Pair(7, 'seven'); // Pair<int, String>, inferred
  print('$pair swapped is ${pair.swap()}');
}
