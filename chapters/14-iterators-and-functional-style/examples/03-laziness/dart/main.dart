// Both functions say when they run, so the output shows the order of the work.
int square(int n) {
  print('  square $n');
  return n * n;
}

bool even(int x) {
  print('  even? $x');
  return x % 2 == 0;
}

// A generator that counts up forever, so only a lazy pipeline can use it.
Iterable<int> naturals() sync* {
  for (var n = 1; ; n++) {
    yield n;
  }
}

void main() {
  final numbers = [1, 2, 3, 4, 5];

  // Eager: toList runs each step over the whole list before the next starts.
  print('eager:');
  final squares = numbers.map(square).toList();
  final evens = squares.where(even).toList();
  print('  first two: ${evens.take(2).join(' ')}');

  // Lazy: map, where, and take on an Iterable are lazy already. toList pulls
  // each number through every step, and take stops after two.
  print('lazy:');
  final firstTwo = numbers.map(square).where(even).take(2).toList();
  print('  first two: ${firstTwo.join(' ')}');

  final endless = naturals()
      .map((n) => n * n)
      .where((x) => x % 2 == 0 && x > 50)
      .take(3);
  print('endless: ${endless.join(' ')}');
}
