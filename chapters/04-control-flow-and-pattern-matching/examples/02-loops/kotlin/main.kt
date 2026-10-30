fun main() {
    var line = "up:"
    for (i in 0..<5) { // 0..<5 is 0 to 4; 0..5 would include the 5
        line += " $i"
    }
    println(line)

    line = "down:"
    for (i in 5 downTo 1) {
        line += " $i"
    }
    println(line)

    line = "by threes:"
    for (i in 0..<10 step 3) {
        line += " $i"
    }
    println(line)

    line = "halving:"
    var n = 100
    while (n > 0) {
        line += " $n"
        n /= 2
    }
    println(line)

    val colors = listOf("red", "green", "blue")
    line = "colors:"
    for (color in colors) {
        line += " $color"
    }
    println(line)

    line = "numbered:"
    for ((i, color) in colors.withIndex()) {
        line += " ${i + 1}=$color" // withIndex gives (index, value) pairs
    }
    println(line)

    // do-while tests the condition after the body, so the body always runs once.
    var m = 100
    do {
        m *= 2
    } while (m < 50)
    println("do-while: $m")
}
