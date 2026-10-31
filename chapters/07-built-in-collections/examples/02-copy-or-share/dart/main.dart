// The function gets a reference to the same list.
void setFirst(List<int> items) {
  items[0] = -1;
}

void main() {
  final a = [1, 2, 3];

  final b = a; // another reference to the same list
  b[0] = 99;
  print('b = a, then b[0] = 99: a is $a');

  final c = [...a]; // or List.of(a)
  c[0] = 0;
  print('c = copy of a, then c[0] = 0: a is $a');

  setFirst(a);
  print('set_first(a) sets a[0] = -1: a is $a');

  final grid = [
    [0, 0],
    [0, 0],
  ];
  final g = [...grid]; // a new outer list holding the same two rows
  g[0][0] = 1;
  print('g = copy of grid, then g[0][0] = 1: grid is $grid');
}
