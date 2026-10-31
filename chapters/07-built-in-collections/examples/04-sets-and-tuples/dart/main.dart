// A set literal makes a LinkedHashSet, which keeps insertion order.
String show(Set<int> items) => (items.toList()..sort()).join(' ');

void main() {
  final a = {1, 2, 3, 4};
  final b = {3, 4, 5};
  print('a: ${show(a)}');
  print('b: ${show(b)}');
  print('union: ${show(a.union(b))}');
  print('intersection: ${show(a.intersection(b))}');
  print('a minus b: ${show(a.difference(b))}');
  print('3 in a: ${a.contains(3) ? 'yes' : 'no'}');

  // A literal like {3, 1, 3} gets a warning for the repeated item, so convert a list.
  print('unique of 3 1 3 2 1, sorted: ${show([3, 1, 3, 2, 1].toSet())}');

  // A record such as (0, 0) is equal to any other record with the same fields, so
  // records work as set members and map keys.
  final path = [(0, 0), (1, 0), (0, 0), (1, 1)];
  final visited = path.toSet();
  print('visited ${path.length} points, ${visited.length} distinct');
  print('(1, 0) visited: ${visited.contains((1, 0)) ? 'yes' : 'no'}');
}
