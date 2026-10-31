// The function gets a reference to the same list.
fun setFirst(items: MutableList<Int>) {
    items[0] = -1
}

fun main() {
    val a = mutableListOf(1, 2, 3)

    val b = a // another reference to the same list
    b[0] = 99
    println("b = a, then b[0] = 99: a is $a")

    val c = a.toMutableList() // a new list with the same items
    c[0] = 0
    println("c = copy of a, then c[0] = 0: a is $a")

    setFirst(a)
    println("set_first(a) sets a[0] = -1: a is $a")

    val grid = listOf(mutableListOf(0, 0), mutableListOf(0, 0))
    val g = grid.toList() // a new outer list holding the same two rows
    g[0][0] = 1
    println("g = copy of grid, then g[0][0] = 1: grid is $grid")
}
