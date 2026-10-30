import java.util.Locale

fun main() {
    while (true) {
        val line = readlnOrNull() ?: break
        val parts = line.trim().split(Regex("\\s+"))
        if (parts.size != 2) continue // a blank line
        val value = parts[0].toDouble()
        // Locale.ROOT keeps the decimal point a "." on machines set to, say, German.
        if (parts[1] == "C") {
            val f = value * 9 / 5 + 32
            println(String.format(Locale.ROOT, "%.1f C = %.1f F", value, f))
        } else {
            val c = (value - 32) * 5 / 9
            println(String.format(Locale.ROOT, "%.1f F = %.1f C", value, c))
        }
    }
}
