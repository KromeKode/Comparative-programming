fun makeStep(op: String, n: Long): (Long) -> Long =
    when (op) {
        "add" -> { x -> x + n }
        "sub" -> { x -> x - n }
        "mul" -> { x -> x * n }
        "min" -> { x -> minOf(x, n) }
        "max" -> { x -> maxOf(x, n) }
        else -> throw IllegalArgumentException("unknown step: $op")
    }

fun main() {
    while (true) {
        val line = readlnOrNull() ?: break
        val words = line.trim().split(Regex("\\s+"))
        if (words[0].isEmpty()) continue // a blank line
        val start = words[0].toLong()
        // chunked(2) pairs up the words after the start: [add, 3], [mul, 2], ...
        val steps = words.drop(1).chunked(2).map { (op, n) -> makeStep(op, n.toLong()) }
        var value = start
        for (step in steps) {
            value = step(value)
        }
        println("$start -> $value")
    }
}
