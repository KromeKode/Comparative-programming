fun yesNo(condition: Boolean) = if (condition) "yes" else "no"

fun main() {
    val a = "hello"
    val b = listOf("hel", "lo").joinToString("") // built separately, same characters
    // == calls equals(), which compares contents for strings, lists, and data classes.
    println("strings with ==: ${yesNo(a == b)}")
    // === asks a different question: are these the same object?
    println("strings with ===: ${yesNo(a === b)}")

    val xs = listOf(1, 2, 3)
    val ys = listOf(1, 2, 3)
    println("lists with ==: ${yesNo(xs == ys)}")

    val nan = Double.NaN
    println("NaN == NaN: ${yesNo(nan == nan)}")
}
