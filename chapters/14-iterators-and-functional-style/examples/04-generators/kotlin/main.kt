// sequence { } builds a Sequence from a block that calls yield. The block is
// a coroutine: it runs only as far as the next yield each time the loop asks
// for an item, and pauses there with its variables kept.
fun fibonacci(): Sequence<Int> = sequence {
    var a = 0
    var b = 1
    while (true) {
        yield(a)
        val next = a + b
        a = b
        b = next
    }
}

data class Node(val value: Int, val left: Node? = null, val right: Node? = null)

// yieldAll hands on everything another sequence yields. Each recursive call
// pauses inside its own block, so the sequence keeps the path down the tree.
fun inOrder(node: Node?): Sequence<Int> = sequence {
    if (node != null) {
        yieldAll(inOrder(node.left))
        yield(node.value)
        yieldAll(inOrder(node.right))
    }
}

fun main() {
    val tree = Node(4, Node(2, Node(1), Node(3)), Node(6, Node(5), Node(7)))
    println("fibonacci: ${fibonacci().take(10).joinToString(" ")}")
    println("in order: ${inOrder(tree).joinToString(" ")}")
    // takeWhile stops asking at the first value over 3, so 5, 6, and 7 are
    // never visited.
    println("up to 3: ${inOrder(tree).takeWhile { it <= 3 }.joinToString(" ")}")
}
