void addFive(List<int> items) {
  items.add(5); // changes the caller's list, because items refers to it
}

String yesNo(bool flag) => flag ? 'yes' : 'no';

// == on two lists asks "same object?", so equal items need a loop.
bool equalItems(List<int> x, List<int> y) {
  if (x.length != y.length) return false;
  for (var i = 0; i < x.length; i++) {
    if (x[i] != y[i]) return false;
  }
  return true;
}

void main() {
  // Every variable holds a reference. Assignment copies the reference.
  final a = [1, 2, 3];
  final b = a;
  b.add(4);
  print('b = a, then b gets 4: a = $a');

  addFive(a);
  print('a function appends 5: a = $a');

  // Numbers and strings cannot change, so sharing them is never visible.
  final n = 1;
  var m = n;
  m += 1;
  print('m = n, then m += 1: n = $n, m = $m');

  // List.of makes a new outer list holding the same inner lists.
  final grid = [
    [1, 2],
    [3, 4],
  ];
  final shallow = List.of(grid);
  shallow[0][0] = 9;
  print('shallow copy, then copy[0][0] = 9: grid = $grid');

  // Dart has no general deep copy, so copy each level by hand.
  final deep = [for (final row in grid) List.of(row)];
  deep[0][0] = 7;
  print('deep copy, then copy[0][0] = 7: grid = $grid');

  // List.filled puts the same list in every slot.
  final rows = List.filled(3, [0, 0]);
  rows[0][0] = 1;
  print('one row in 3 slots, then rows[0][0] = 1: rows = $rows');

  final same = List.of(a);
  print('a and b are the same list: ${yesNo(identical(a, b))}');
  print('a and its copy are the same list: ${yesNo(identical(a, same))}');
  print('a and its copy hold equal items: ${yesNo(equalItems(a, same))}');
}
