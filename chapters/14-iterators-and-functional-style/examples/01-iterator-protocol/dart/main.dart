// A ring keeps the last three items pushed into it, oldest first. Extending
// Iterable means writing only the iterator getter; for-in, map, where, join,
// and the rest come with it.
class Ring extends Iterable<int> {
  final _items = [0, 0, 0];
  var _start = 0;
  var _len = 0;

  void push(int item) {
    _items[(_start + _len) % 3] = item;
    if (_len < 3) {
      _len++;
    } else {
      _start = (_start + 1) % 3; // the oldest item is overwritten
    }
  }

  @override
  Iterator<int> get iterator => _RingIterator(_items, _start, _len);
}

// A Dart iterator has two members: moveNext() steps forward and says whether
// there is an item, and current is that item.
class _RingIterator implements Iterator<int> {
  final List<int> _items;
  final int _start;
  final int _len;
  var _index = -1;

  _RingIterator(this._items, this._start, this._len);

  @override
  bool moveNext() => ++_index < _len;

  @override
  int get current => _items[(_start + _index) % 3];
}

void main() {
  final ring = Ring();
  for (var i = 1; i <= 5; i++) {
    ring.push(i);
  }

  var line = 'loop:';
  for (final item in ring) {
    line += ' $item';
  }
  print(line);
  print('again: ${ring.join(' ')}'); // join asks for a new iterator too

  var it = ring.iterator;
  final parts = <int>[];
  while (it.moveNext()) {
    parts.add(it.current);
  }
  print('by hand: ${parts.join(', then ')}, then nothing');

  // for-in takes an Iterable, not an Iterator, so carrying on from where an
  // iterator is means calling moveNext.
  it = ring.iterator;
  it.moveNext();
  line = 'resumed: took ${it.current}, then the loop got';
  while (it.moveNext()) {
    line += ' ${it.current}';
  }
  print(line);
}
