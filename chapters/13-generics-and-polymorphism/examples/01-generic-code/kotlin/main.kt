// A generic class. T is a type parameter: each use of Stack fills it in.
class Stack<T> {
    private val items = mutableListOf<T>()

    fun push(item: T) {
        items.add(item)
    }

    fun pop(): T? = items.removeLastOrNull()

    fun peek(): T? = items.lastOrNull()

    val size: Int
        get() = items.size
}

// A generic function. The bound T : Comparable<T> lets the body use >, and
// only types that can be compared may be passed in.
fun <T : Comparable<T>> largest(items: List<T>): T? {
    var best = items.firstOrNull() ?: return null
    for (item in items) {
        if (item > best) best = item
    }
    return best
}

// Kotlin already has Pair<A, B>. A generic extension function adds swap to it.
fun <A, B> Pair<A, B>.swap(): Pair<B, A> = Pair(second, first)

fun <T> show(value: T?): String = value?.toString() ?: "none"

fun <T> report(name: String, stack: Stack<T>) {
    val popped = show(stack.pop())
    val top = show(stack.peek())
    println("$name: popped $popped, top is $top, ${stack.size} left")
}

fun main() {
    val numbers = Stack<Int>()
    numbers.push(3)
    numbers.push(1)
    numbers.push(4)
    report("numbers", numbers)

    val words = Stack<String>()
    words.push("pear")
    words.push("fig")
    report("words", words)

    val empty = Stack<Double>()
    report("empty", empty)

    println("largest number: ${show(largest(listOf(3, 1, 4, 1, 5)))}")
    println("largest word: ${show(largest(listOf("pear", "fig", "apple")))}")
    println("largest of nothing: ${show(largest(emptyList<Int>()))}")

    val pair = Pair(7, "seven") // Pair<Int, String>, inferred
    println("$pair swapped is ${pair.swap()}")
}
