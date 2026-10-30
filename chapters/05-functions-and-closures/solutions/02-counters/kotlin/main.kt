fun makeCounter(start: Long, step: Long): () -> Long {
    var next = start // captured by the lambda, and kept alive by it
    return {
        val value = next
        next += step
        value
    }
}

fun main() {
    val counters = mutableMapOf<String, () -> Long>()
    while (true) {
        val line = readlnOrNull() ?: break
        val words = line.trim().split(Regex("\\s+"))
        when {
            words.size == 4 && words[0] == "new" -> {
                val (_, name, start, step) = words // unpack a list by position
                counters[name] = makeCounter(start.toLong(), step.toLong())
            }
            words.size == 2 && words[0] == "next" -> {
                val name = words[1]
                val counter = counters[name]
                println(if (counter == null) "$name: no such counter" else "$name: ${counter()}")
            }
        }
    }
}
