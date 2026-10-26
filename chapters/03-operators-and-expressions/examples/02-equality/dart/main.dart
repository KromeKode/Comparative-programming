String yesNo(bool condition) => condition ? 'yes' : 'no';

void main() {
  final a = 'hello';
  final b = ['hel', 'lo'].join(); // built separately, same characters
  print('strings with ==: ${yesNo(a == b)}');

  // A List's == asks whether the two lists are the same object.
  final xs = [1, 2, 3];
  final ys = [1, 2, 3];
  print('lists with ==: ${yesNo(xs == ys)}');
  // Equal const values are created once, at compile time, so they are one object.
  print('const lists with ==: ${yesNo(const [1, 2, 3] == const [1, 2, 3])}');
  var sameItems = xs.length == ys.length;
  for (var i = 0; sameItems && i < xs.length; i++) {
    sameItems = xs[i] == ys[i];
  }
  print('lists element by element: ${yesNo(sameItems)}');

  final nan = double.nan;
  print('NaN == NaN: ${yesNo(nan == nan)}');
}
