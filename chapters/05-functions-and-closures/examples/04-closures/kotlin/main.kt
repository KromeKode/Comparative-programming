fun makeAdder(n: Int): (Int) -> Int = { x -> x + n }

fun makeAccumulator(): (Int) -> Int {
    var total = 0
    return { x ->
        total += x // a lambda can change a var it captured
        total // the last line of a lambda is its value
    }
}

fun main() {
    val add5 = makeAdder(5)
    val add10 = makeAdder(10)
    println("add5(10) = ${add5(10)}")
    println("add10(10) = ${add10(10)}")

    val running = makeAccumulator()
    println("running total: ${running(3)} ${running(5)} ${running(2)}")

    val made = mutableListOf<() -> Int>()
    for (i in 0..<3) {
        made.add { i } // each pass of a for loop has its own i
    }
    println("made in a loop: ${made.map { it() }.joinToString(" ")}")
}
