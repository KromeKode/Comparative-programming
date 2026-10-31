void main() {
  final numbers = [3, 1, 4, 1, 5]; // a List<int>, which can grow
  print('numbers: $numbers'); // a List prints as [3, 1, 4, 1, 5]
  print('length: ${numbers.length}');
  print('first: ${numbers.first}, last: ${numbers.last}');

  numbers.add(9);
  print('append 9: $numbers');
  numbers.insert(1, 2);
  print('insert 2 at index 1: $numbers');
  final removed = numbers.removeAt(0);
  print('remove index 0 (was $removed): $numbers');

  print('contains 4: ${numbers.contains(4) ? 'yes' : 'no'}');
  print('index of 1: ${numbers.indexOf(1)}'); // -1 if it is missing

  numbers.sort();
  print('sorted: $numbers');
  print('slice 1..4: ${numbers.sublist(1, 4)}'); // a new list, copied

  final reversed = numbers.reversed.toList(); // reversed is a lazy view
  print('reversed: $reversed');
}
