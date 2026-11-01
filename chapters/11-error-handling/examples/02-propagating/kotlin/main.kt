// Pretend files, so the example does not touch the disk.
val files = mapOf("good.txt" to "1\n2\n3", "bad.txt" to "4\nfive\n6")

// One exception class per kind of failure, so callers can catch just one.
class NotFound(message: String) : Exception(message)

class NotANumber(message: String) : Exception(message)

class BadLine(message: String, cause: Throwable) : Exception(message, cause)

fun readFile(name: String): String = files[name] ?: throw NotFound("$name: no such file")

fun parseNumber(text: String): Int =
    text.toIntOrNull() ?: throw NotANumber("\"$text\" is not a number")

fun sumFile(name: String): Int {
    val text = readFile(name) // a NotFound passes straight through
    var total = 0
    for ((index, line) in text.lines().withIndex()) {
        try {
            total += parseNumber(line)
        } catch (e: NotANumber) {
            // Throw a new exception and keep the old one as its cause.
            throw BadLine("$name: line ${index + 1}", e)
        }
    }
    return total
}

// A missing file counts as 0, but any other error stops the sum.
fun sumAll(names: List<String>): Int {
    var total = 0
    for (name in names) {
        try {
            total += sumFile(name)
        } catch (_: NotFound) {}
    }
    return total
}

// Join an exception's message with the messages of its causes.
fun describe(error: Throwable): String =
    generateSequence(error) { it.cause }.joinToString(": ") { it.message ?: "" }

fun show(label: String, run: () -> Int) {
    try {
        println("  $label -> ${run()}")
    } catch (e: Exception) {
        println("  $label -> error: ${describe(e)}")
    }
}

fun main() {
    println("one file at a time:")
    for (name in listOf("good.txt", "bad.txt", "missing.txt")) {
        show(name) { sumFile(name) }
    }
    println("several files, where a missing file counts as 0:")
    show("good.txt + missing.txt") { sumAll(listOf("good.txt", "missing.txt")) }
    show("good.txt + bad.txt") { sumAll(listOf("good.txt", "bad.txt")) }
}
