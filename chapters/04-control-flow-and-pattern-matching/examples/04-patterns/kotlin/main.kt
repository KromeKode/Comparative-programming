fun classify(point: Pair<Int, Int>): String {
    // when cannot take a pair apart, so destructure it first.
    val (x, y) = point
    // A when with no subject tests each condition in order, like an else-if chain.
    return when {
        x == 0 && y == 0 -> "origin"
        y == 0 -> "on the x axis at x = $x"
        x == 0 -> "on the y axis at y = $y"
        x == y -> "on the diagonal at $x"
        else -> "x = $x, y = $y"
    }
}

fun main() {
    for ((x, y) in listOf(0 to 0, 3 to 0, 0 to -2, 4 to 4, -1 to 5)) {
        println("($x, $y): ${classify(x to y)}")
    }
}
