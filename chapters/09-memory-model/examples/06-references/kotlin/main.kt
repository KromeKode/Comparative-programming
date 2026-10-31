fun addFive(items: MutableList<Int>) {
    items.add(5) // changes the caller's list, because items refers to it
}

fun yesNo(flag: Boolean) = if (flag) "yes" else "no"

fun main() {
    // Every object is reached through a reference. Assignment copies the reference.
    val a = mutableListOf(1, 2, 3)
    val b = a
    b.add(4)
    println("b = a, then b gets 4: a = $a")

    addFive(a)
    println("a function appends 5: a = $a")

    // Int is stored as a plain number where it can be, and it cannot change.
    val n = 1
    var m = n
    m += 1
    println("m = n, then m += 1: n = $n, m = $m")

    // toMutableList makes a new outer list holding the same inner lists.
    val grid = mutableListOf(mutableListOf(1, 2), mutableListOf(3, 4))
    val shallow = grid.toMutableList()
    shallow[0][0] = 9
    println("shallow copy, then copy[0][0] = 9: grid = $grid")

    // Kotlin has no general deep copy, so copy each level by hand.
    val deep = grid.map { it.toMutableList() }
    deep[0][0] = 7
    println("deep copy, then copy[0][0] = 7: grid = $grid")

    // The lambda returns the same row every time.
    val row = mutableListOf(0, 0)
    val rows = MutableList(3) { row }
    rows[0][0] = 1
    println("one row in 3 slots, then rows[0][0] = 1: rows = $rows")

    // === asks "same object?"; == asks "equal items?".
    val same = a.toMutableList()
    println("a and b are the same list: ${yesNo(a === b)}")
    println("a and its copy are the same list: ${yesNo(a === same)}")
    println("a and its copy hold equal items: ${yesNo(a == same)}")
}
