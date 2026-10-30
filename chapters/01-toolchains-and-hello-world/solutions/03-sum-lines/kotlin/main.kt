fun main() {
    var total = 0L // the L makes it a Long, 64 bits; a plain 0 would be a 32-bit Int
    while (true) {
        val line = readlnOrNull() ?: break // end of input
        val trimmed = line.trim()
        if (trimmed.isEmpty()) continue
        total += trimmed.toLong()
    }
    println(total)
}
