fun isLeap(year: Int) = year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)

fun main() {
    while (true) {
        val line = readlnOrNull() ?: break
        val text = line.trim()
        if (text.isEmpty()) continue
        val year = text.toInt()
        println("$year: ${if (isLeap(year)) "leap year" else "common year"}")
    }
}
