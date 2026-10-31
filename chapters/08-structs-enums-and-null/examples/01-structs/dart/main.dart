// A primary constructor (Dart 3.13) declares the fields and the constructor at once.
class Point(var int x, var int y) {
  int stepsFromOrigin() => x.abs() + y.abs();

  // Dart has no built-in "copy with changes", so classes write their own.
  Point copyWith({int? x, int? y}) => Point(x ?? this.x, y ?? this.y);

  // Without these two, == would ask whether both are the same object.
  @override
  bool operator ==(Object other) =>
      other is Point && other.x == x && other.y == y;

  @override
  int get hashCode => Object.hash(x, y);

  @override
  String toString() => '($x, $y)';
}

void main() {
  final p = Point(1, 2);
  print('p = $p');
  print('p.x + p.y = ${p.x + p.y}');

  final q = p.copyWith(x: 5);
  print('q = p with x = 5: q = $q, p = $p');

  // Not a copy: r and p refer to the same object.
  final r = p;
  r.y = 9;
  print('r = p, then r.y = 9: r = $r, p = $p');

  final a = Point(3, -4);
  final b = Point(3, -4);
  print('a = $a, b = $b, a == b: ${a == b ? 'yes' : 'no'}');
  print('steps from (0, 0) to a: ${a.stepsFromOrigin()}');
}
