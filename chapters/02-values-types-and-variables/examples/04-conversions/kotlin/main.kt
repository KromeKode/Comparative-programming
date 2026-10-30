import kotlin.math.roundToInt

fun main() {
    val parsed = "42".toInt() // throws NumberFormatException on bad input
    println("\"42\" + 1 as a number: ${parsed + 1}")

    val x = 3.99
    println("3.99 truncated: ${x.toInt()}")
    println("-3.99 truncated: ${(-x).toInt()}")
    println("2.5 rounded: ${2.5.roundToInt()}") // halfway rounds toward positive infinity
    println("-2.5 rounded: ${(-2.5).roundToInt()}")

    val seven = 7
    val two = 2
    println("7 / 2 as integers: ${seven / two}")
    println("7 / 2 as floats: ${seven / two.toDouble()}") // Int / Double gives a Double
}
