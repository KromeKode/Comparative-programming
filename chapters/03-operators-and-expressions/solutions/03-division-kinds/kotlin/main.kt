fun main() {
    while (true) {
        val line = readlnOrNull() ?: break
        val parts = line.trim().split(Regex("\\s+"))
        if (parts.size != 2) continue // a blank line
        val a = parts[0].toInt()
        val b = parts[1].toInt()
        // Kotlin has both kinds built in: / and % truncate, floorDiv and mod round down.
        println("$a $b -> truncated ${a / b} ${a % b}, floored ${a.floorDiv(b)} ${a.mod(b)}")
    }
}
