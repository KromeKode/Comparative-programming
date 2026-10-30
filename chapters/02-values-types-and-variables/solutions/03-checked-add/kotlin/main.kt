fun main() {
    while (true) {
        val line = readlnOrNull() ?: break
        val parts = line.trim().split(Regex("\\s+"))
        if (parts.size != 2) continue // a blank line
        val a = parts[0].toLong()
        val b = parts[1].toLong()
        // Plain a + b would wrap. Math.addExact throws on overflow instead.
        val sum =
            try {
                Math.addExact(a, b)
            } catch (e: ArithmeticException) {
                null
            }
        println(sum ?: "overflow")
    }
}
