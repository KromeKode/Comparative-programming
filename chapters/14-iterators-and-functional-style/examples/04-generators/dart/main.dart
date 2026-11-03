// sync* makes a generator function: it returns an Iterable, and the body runs
// only as far as the next yield each time the loop asks for an item.
Iterable<int> fibonacci() sync* {
  var (a, b) = (0, 1);
  while (true) {
    yield a;
    (a, b) = (b, a + b);
  }
}

class Node {
  final int value;
  final Node? left, right;

  const Node(this.value, [this.left, this.right]);
}

// yield* hands on everything another Iterable yields. Each recursive call
// pauses inside its own frame, so the generator keeps the path down the tree.
Iterable<int> inOrder(Node? node) sync* {
  if (node == null) return;
  yield* inOrder(node.left);
  yield node.value;
  yield* inOrder(node.right);
}

void main() {
  const tree = Node(4, Node(2, Node(1), Node(3)), Node(6, Node(5), Node(7)));
  print('fibonacci: ${fibonacci().take(10).join(' ')}');
  print('in order: ${inOrder(tree).join(' ')}');
  // takeWhile stops asking at the first value over 3, so 5, 6, and 7 are
  // never visited.
  print('up to 3: ${inOrder(tree).takeWhile((value) => value <= 3).join(' ')}');
}
