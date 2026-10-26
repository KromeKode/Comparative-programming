var calls = 0; // counts how many times the right side ran

bool bigRatio(int x) {
  calls++;
  return 10 ~/ x > 1; // dividing by zero here would throw
}

String displayName(String name) {
  // Conditions must be bool, so test isEmpty explicitly.
  return name.isEmpty ? 'stranger' : name;
}

void main() {
  for (final x in [0, 5]) {
    // && stops as soon as the left side is false, so bigRatio never sees 0.
    final ok = x != 0 && bigRatio(x);
    print('x = $x: ${ok ? 'yes' : 'no'}');
  }
  print('right side ran $calls of 2 times');
  print('name "": ${displayName('')}');
  print('name "Ada": ${displayName('Ada')}');
}
