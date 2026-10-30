import 'dart:io';

final memo = <int, int>{};

int fib(int n) {
  if (n < 2) return n;
  // putIfAbsent runs the function only when n is not in the map yet.
  // Dart allows that function to add other keys, as the recursion does.
  return memo.putIfAbsent(n, () => fib(n - 1) + fib(n - 2));
}

void main() {
  while (true) {
    final line = stdin.readLineSync();
    if (line == null) break;
    final text = line.trim();
    if (text.isEmpty) continue;
    final n = int.parse(text);
    print('fib($n) = ${fib(n)}');
  }
}
