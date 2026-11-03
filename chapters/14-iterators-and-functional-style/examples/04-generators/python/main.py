from collections.abc import Iterator
from dataclasses import dataclass
from itertools import islice, takewhile


# A function with yield in it is a generator function. Calling it runs none of
# the body: it returns a generator, and each next() runs the body to the next
# yield, where it pauses with all its variables kept.
def fibonacci() -> Iterator[int]:
    a, b = 0, 1
    while True:
        yield a
        a, b = b, a + b


@dataclass
class Node:
    value: int
    left: "Node | None" = None
    right: "Node | None" = None


# yield from hands on everything another generator yields. Each recursive call
# pauses inside its own frame, so the generator keeps the path down the tree.
def in_order(node: Node | None) -> Iterator[int]:
    if node is None:
        return
    yield from in_order(node.left)
    yield node.value
    yield from in_order(node.right)


def main() -> None:
    tree = Node(4, Node(2, Node(1), Node(3)), Node(6, Node(5), Node(7)))
    print("fibonacci:", *islice(fibonacci(), 10))
    print("in order:", *in_order(tree))
    # takewhile stops asking at the first value over 3, so 5, 6, and 7 are
    # never visited.
    print("up to 3:", *takewhile(lambda value: value <= 3, in_order(tree)))


if __name__ == "__main__":
    main()
