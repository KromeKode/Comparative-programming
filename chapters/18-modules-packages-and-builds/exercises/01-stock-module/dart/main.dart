// Exercise 18.1: a stock module. This file is given: write stock.dart so that it
// runs. It reads the commands, and leaves keeping count to the library.
import 'dart:io';

import 'stock.dart';

// An amount is a whole number from 1 to 999,999,999,999,999.
final amountPattern = RegExp(r'^0*[1-9][0-9]{0,14}$');

void main() {
  final stock = Stock();
  var number = 0;
  for (String? raw; (raw = stdin.readLineSync()) != null;) {
    number++;
    final words = raw!
        .split(RegExp(r'\s+'))
        .where((word) => word.isNotEmpty)
        .toList();
    if (words.isEmpty) continue;
    final amount = words.length == 3 && amountPattern.hasMatch(words[2])
        ? int.parse(words[2])
        : null;
    switch (words) {
      case ['add', final item, _] when amount != null:
        stock.add(item, amount);
        print('added $amount $item, now ${stock.count(item)}');
      case ['take', final item, _] when amount != null:
        if (stock.take(item, amount)) {
          print('took $amount $item, now ${stock.count(item)}');
        } else {
          print(
            'error: line $number: only ${stock.count(item)} $item in stock',
          );
        }
      case ['count', final item]:
        print('$item: ${stock.count(item)}');
      case ['list']:
        final items = [
          for (final (name, count) in stock.items()) '$name $count',
        ];
        print('stock: ${items.isEmpty ? 'empty' : items.join(', ')}');
      default:
        print('error: line $number: expected add, take, count, or list');
    }
  }
}
