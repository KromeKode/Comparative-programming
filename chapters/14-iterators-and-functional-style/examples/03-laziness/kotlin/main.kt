// Both functions say when they run, so the output shows the order of the work.
fun square(n: Int): Int {
    println("  square $n")
    return n * n
}

fun even(x: Int): Boolean {
    println("  even? $x")
    return x % 2 == 0
}

fun main() {
    val numbers = listOf(1, 2, 3, 4, 5)

    // Eager: List operations build a whole new list at each step.
    println("eager:")
    val evens = numbers.map(::square).filter(::even)
    println("  first two: ${evens.take(2).joinToString(" ")}")

    // Lazy: asSequence switches to Sequence operations, which pull one item at
    // a time through every step. toList starts the pulling.
    println("lazy:")
    val firstTwo = numbers.asSequence().map(::square).filter(::even).take(2).toList()
    println("  first two: ${firstTwo.joinToString(" ")}")

    // generateSequence counts up forever, so only a lazy pipeline can use it.
    val endless =
        generateSequence(1) { it + 1 }.map { it * it }.filter { it % 2 == 0 && it > 50 }.take(3)
    println("endless: ${endless.joinToString(" ")}")
}
