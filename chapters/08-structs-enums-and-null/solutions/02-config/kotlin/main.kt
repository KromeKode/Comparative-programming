// null means "not set", which is different from every real value.
data class Config(var host: String? = null, var port: Int? = null, var timeout: Int? = null)

// The number if text is digits only and from 1 to high, or else null.
fun parseNumber(text: String, high: Int): Int? {
    if (text.isEmpty() || !text.all { it in '0'..'9' }) return null
    // toIntOrNull gives null when the number is too big for an Int.
    return text.toIntOrNull()?.takeIf { it in 1..high }
}

fun main() {
    val config = Config()
    var number = 0
    while (true) {
        val line = readlnOrNull()?.trim() ?: break
        number++
        if (line.isEmpty() || line.startsWith("#")) continue
        if ('=' !in line) {
            println("line $number: expected key = value")
            continue
        }
        val key = line.substringBefore('=').trim()
        val value = line.substringAfter('=').trim()
        when (key) {
            "host" -> config.host = value.ifEmpty { null }
            "port",
            "timeout" -> {
                val parsed = parseNumber(value, if (key == "port") 65535 else 86400)
                if (value.isNotEmpty() && parsed == null) {
                    println("line $number: bad $key \"$value\"")
                } else if (key == "port") {
                    config.port = parsed
                } else {
                    config.timeout = parsed
                }
            }
            else -> println("line $number: unknown key \"$key\"")
        }
    }

    println("host: ${config.host ?: "localhost (default)"}")
    println("port: ${config.port ?: "8080 (default)"}")
    println("timeout: ${config.timeout?.let { "${it}s" } ?: "none"}")
}
