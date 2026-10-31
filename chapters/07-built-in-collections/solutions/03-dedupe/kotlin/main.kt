fun main() {
    while (true) {
        val line = readlnOrNull() ?: break
        val words = line.split(Regex("\\s+")).filter { it.isNotEmpty() }
        if (words.isEmpty()) continue
        println(words.distinct().joinToString(" ")) // distinct keeps first occurrences
    }
}
