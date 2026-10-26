int addOne(int x) => x + 1;
int triple(int x) => x * 3;

// int Function(int) is the type of any function from int to int.
int applyTwice(int Function(int) f, int x) => f(f(x));

int Function(int) compose(int Function(int) outer, int Function(int) inner) =>
    (x) => outer(inner(x));

void main() {
  print('apply_twice(add_one, 5) = ${applyTwice(addOne, 5)}');
  print('apply_twice(triple, 5) = ${applyTwice(triple, 5)}');
  print('apply_twice(square, 3) = ${applyTwice((x) => x * x, 3)}');
  final addThenTriple = compose(triple, addOne);
  print('compose(triple, add_one)(5) = ${addThenTriple(5)}');
}
