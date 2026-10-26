int Function(int) makeAdder(int n) =>
    (x) => x + n;

int Function(int) makeAccumulator() {
  var total = 0;
  return (x) {
    total += x;
    return total;
  };
}

void main() {
  final add5 = makeAdder(5);
  final add10 = makeAdder(10);
  print('add5(10) = ${add5(10)}');
  print('add10(10) = ${add10(10)}');

  final running = makeAccumulator();
  print('running total: ${running(3)} ${running(5)} ${running(2)}');

  final made = <int Function()>[];
  for (var i = 0; i < 3; i++) {
    made.add(() => i); // a for loop gives each pass a fresh i
  }
  print('made in a loop: ${made.map((f) => f()).join(' ')}');
}
