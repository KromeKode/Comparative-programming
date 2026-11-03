data class Point(val x: Int, val y: Int)

// Overloads: one name, a separate function for each type. The compiler picks
// one from the type of the argument.
fun show(value: Int) = value.toString()

fun show(value: Boolean) = value.toString()

fun show(value: String) = "\"$value\""

fun show(value: Point) = "(${value.x}, ${value.y})"

// Inside a generic function there is no show(item) for an unknown T, so the
// caller passes in the one to use.
fun <T> show(items: List<T>, showItem: (T) -> String) =
    items.joinToString(", ", "[", "]", transform = showItem)

fun main() {
    println(show(42))
    println(show("hi"))
    println(show(true))
    println(show(listOf(1, 2, 3), ::show))
    println(show(listOf(listOf("a", "b"), listOf(), listOf("c"))) { show(it, ::show) })
    println(show(Point(1, 2)))
    println(show(listOf(Point(1, 2), Point(3, 4)), ::show))
}
