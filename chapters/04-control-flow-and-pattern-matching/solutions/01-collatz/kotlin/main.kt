fun main() {
    while (true) {
        val line = readlnOrNull() ?: break
        val text = line.trim()
        if (text.isEmpty()) continue
        val start = text.toLong() // the peak can pass 2^31, so use Long
        var n = start
        var peak = n
        var steps = 0
        while (n != 1L) {
            n = if (n % 2 == 0L) n / 2 else 3 * n + 1 // if is an expression
            peak = maxOf(peak, n)
            steps++
        }
        println("$start: steps $steps, peak $peak")
    }
}
