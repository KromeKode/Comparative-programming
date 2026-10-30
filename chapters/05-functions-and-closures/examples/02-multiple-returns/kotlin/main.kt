// Pair holds two values without declaring a class.
fun minMax(values: List<Int>): Pair<Int, Int> {
    var lowest = values[0]
    var highest = values[0]
    for (value in values) {
        if (value < lowest) lowest = value
        if (value > highest) highest = value
    }
    return Pair(lowest, highest)
}

// A data class names the values, and still unpacks like a Pair.
data class DivMod(val quotient: Int, val remainder: Int)

fun divMod(a: Int, b: Int) = DivMod(a / b, a % b)

fun main() {
    val (lowest, highest) = minMax(listOf(3, -1, 4, 1, 5))
    println("min and max of [3, -1, 4, 1, 5]: $lowest $highest")

    val (quotient, remainder) = divMod(17, 5) // unpacks by position, not by name
    println("17 divided by 5: $quotient remainder $remainder")

    var first = "hello"
    var second = "world"
    first = second.also { second = first } // also returns second's old value
    println("swapped: $first $second")
}
