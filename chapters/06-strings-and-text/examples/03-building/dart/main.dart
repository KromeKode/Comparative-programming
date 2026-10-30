void main() {
  const fruits = ['apple', 'banana', 'cherry'];
  print('joined: ${fruits.join(', ')}');

  // Strings are immutable. StringBuffer collects pieces and builds the string once.
  final built = StringBuffer();
  for (var n = 1; n <= 5; n++) {
    if (n > 1) built.write('-');
    built.write(n);
  }
  print('built: $built');

  print('padded: [${'Ada'.padRight(8)}] [${'90'.padLeft(8)}]');
  print('zero-padded: ${'42'.padLeft(5, '0')}');
  print('two decimals: ${3.14159.toStringAsFixed(2)}');
}
