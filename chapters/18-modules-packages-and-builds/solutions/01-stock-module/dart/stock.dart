// The stock library: how many of each item there are.

class Stock {
  // A name that starts with _ is private to its library, which is this file:
  // main.dart can't use stock._counts.
  final _counts = <String, int>{};

  void add(String item, int amount) {
    _counts[item] = count(item) + amount;
  }

  /// Takes [amount] of [item] away, and returns true, if there are that many.
  bool take(String item, int amount) {
    if (count(item) < amount) return false;
    _counts[item] = count(item) - amount;
    return true;
  }

  /// How many of [item] there are: 0 for an item never added.
  int count(String item) => _counts[item] ?? 0;

  /// The items there are any of, with their counts, in alphabetical order.
  List<(String, int)> items() {
    final names = _counts.keys.where((name) => _counts[name]! > 0).toList()
      ..sort();
    return [for (final name in names) (name, _counts[name]!)];
  }
}
