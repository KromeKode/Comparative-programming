// setOf makes a LinkedHashSet, which keeps insertion order.
fun show(items: Set<Int>) = items.sorted().joinToString(" ")

fun main() {
    val a = setOf(1, 2, 3, 4)
    val b = setOf(3, 4, 5)
    println("a: ${show(a)}")
    println("b: ${show(b)}")
    println("union: ${show(a union b)}")
    println("intersection: ${show(a intersect b)}")
    println("a minus b: ${show(a subtract b)}")
    println("3 in a: ${if (3 in a) "yes" else "no"}")

    println("unique of 3 1 3 2 1, sorted: ${show(listOf(3, 1, 3, 2, 1).toSet())}")

    // A Pair compares and hashes by its two values, so pairs work as set members.
    val path = listOf(0 to 0, 1 to 0, 0 to 0, 1 to 1)
    val visited = path.toSet()
    println("visited ${path.size} points, ${visited.size} distinct")
    println("(1, 0) visited: ${if ((1 to 0) in visited) "yes" else "no"}")
}
