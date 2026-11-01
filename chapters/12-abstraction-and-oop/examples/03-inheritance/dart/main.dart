// A bag of items. addAll is written in terms of add.
class Bag {
  final List<String> _items = [];

  void add(String item) {
    _items.add(item);
  }

  void addAll(List<String> items) {
    for (final item in items) {
      add(item); // calls the subclass's add, if it has one
    }
  }

  int get size => _items.length;
}

// Inheritance: a CountingBag is a Bag that also counts what was added.
class InheritedCountingBag extends Bag {
  int count = 0;

  @override
  void add(String item) {
    count++;
    super.add(item);
  }

  @override
  void addAll(List<String> items) {
    count += items.length;
    super.addAll(items); // which calls add, which counts again
  }
}

// Composition: a CountingBag has a Bag, and passes the work on to it.
class ComposedCountingBag {
  final Bag _bag = Bag();
  int count = 0;

  void add(String item) {
    count++;
    _bag.add(item);
  }

  void addAll(List<String> items) {
    count += items.length;
    _bag.addAll(items); // the inner Bag calls its own add
  }

  int get size => _bag.size;
}

void report(String label, int size, int count) {
  print('$label: the bag holds $size items, the count says $count');
}

void main() {
  final fruit = ['apple', 'pear', 'plum'];

  final inherited = InheritedCountingBag();
  inherited.addAll(fruit);
  inherited.add('fig');
  report('inheritance', inherited.size, inherited.count);

  final composed = ComposedCountingBag();
  composed.addAll(fruit);
  composed.add('fig');
  report('composition', composed.size, composed.count);
}
