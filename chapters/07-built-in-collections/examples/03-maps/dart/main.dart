// A map literal makes a LinkedHashMap, which remembers the order keys were added in.
String show(Map<String, int> stock) =>
    stock.entries.map((entry) => '${entry.key}=${entry.value}').join(' ');

void main() {
  final stock = {'apple': 3, 'banana': 5};
  print('stock: ${show(stock)}');

  stock['cherry'] = 7;
  print('add cherry: ${show(stock)}');

  print('banana: ${stock['banana']}');
  if (!stock.containsKey('durian')) {
    // [] gives null for a missing key, and ?? swaps in a default.
    print('durian: ${stock['durian'] ?? 0} (missing)');
  }

  stock.update('apple', (count) => count + 2);
  print('apple += 2: ${show(stock)}');
  stock.remove('banana');
  print('remove banana: ${show(stock)}');

  print('has cherry: ${stock.containsKey('cherry') ? 'yes' : 'no'}');
  print('size: ${stock.length}');
}
