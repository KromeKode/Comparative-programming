import kotlin.math.abs

// A data class gets equals, hashCode, toString, and copy written for it.
data class Point(var x: Int, var y: Int) {
    fun stepsFromOrigin() = abs(x) + abs(y)

    override fun toString() = "($x, $y)"
}

fun main() {
    val p = Point(1, 2)
    println("p = $p")
    println("p.x + p.y = ${p.x + p.y}")

    // copy() makes a new Point, with the named fields changed.
    val q = p.copy(x = 5)
    println("q = p with x = 5: q = $q, p = $p")

    // Not a copy: r and p refer to the same object.
    val r = p
    r.y = 9
    println("r = p, then r.y = 9: r = $r, p = $p")

    // == calls equals, which a data class writes to compare every field.
    val a = Point(3, -4)
    val b = Point(3, -4)
    println("a = $a, b = $b, a == b: ${if (a == b) "yes" else "no"}")
    println("steps from (0, 0) to a: ${a.stepsFromOrigin()}")
}
