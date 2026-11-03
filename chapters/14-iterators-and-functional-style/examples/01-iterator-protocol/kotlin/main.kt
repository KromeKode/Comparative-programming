// A ring keeps the last three items pushed into it, oldest first.
class Ring : Iterable<Int> {
    private val items = IntArray(3)
    private var start = 0
    private var len = 0

    fun push(item: Int) {
        items[(start + len) % 3] = item
        if (len < 3) {
            len++
        } else {
            start = (start + 1) % 3 // the oldest item is overwritten
        }
    }

    // A for loop calls iterator(). A Kotlin iterator has hasNext() and next().
    override fun iterator(): Iterator<Int> =
        object : Iterator<Int> {
            private var index = 0

            override fun hasNext() = index < len

            override fun next(): Int {
                if (!hasNext()) throw NoSuchElementException()
                return items[(start + index++) % 3]
            }
        }
}

fun main() {
    val ring = Ring()
    for (i in 1..5) ring.push(i)

    print("loop:")
    for (item in ring) print(" $item")
    println("\nagain: ${ring.joinToString(" ")}") // joinToString asks for a new iterator

    var it = ring.iterator()
    val parts = mutableListOf<Int>()
    while (it.hasNext()) parts.add(it.next())
    println("by hand: ${parts.joinToString(", then ")}, then nothing")

    it = ring.iterator()
    print("resumed: took ${it.next()}, then the loop got")
    for (item in it) print(" $item") // a for loop accepts an Iterator too, and carries on
    println()
}
