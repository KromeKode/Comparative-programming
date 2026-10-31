// A function cannot reach the caller's variables, only the objects they refer
// to, so swap takes the list and two indexes.
fun swap(words: MutableList<String>, i: Int, j: Int) {
    words[i] = words[j].also { words[j] = words[i] }
}

// Reverses words[start..end] and returns how many swaps that took.
fun reverse(words: MutableList<String>, start: Int, end: Int): Int {
    var low = start
    var high = end
    var swaps = 0
    while (low < high) {
        swap(words, low++, high--)
        swaps++
    }
    return swaps
}

fun parsePosition(text: String, count: Int): Int? {
    if (text.isEmpty() || !text.all { it in '0'..'9' }) {
        println("error: \"$text\" is not a position")
        return null
    }
    // toIntOrNull gives null for a number too big for an Int: out of range too.
    val value = text.toIntOrNull()
    if (value == null || value !in 1..count) {
        println("error: position $text is out of range (1 to $count)")
        return null
    }
    return value
}

fun fields(line: String) = line.split(Regex("\\s+")).filter { it.isNotEmpty() }

fun main() {
    val words = readlnOrNull()?.let(::fields)?.toMutableList() ?: mutableListOf()
    var swaps = 0
    while (true) {
        val line = readlnOrNull() ?: break
        val parts = fields(line)
        if (parts.isEmpty()) continue
        val command = parts[0]
        if (command != "swap" && command != "reverse") {
            println("error: unknown command \"$command\"")
            continue
        }
        if (parts.size != 3) {
            println("error: $command needs 2 positions, got ${parts.size - 1}")
            continue
        }
        val i = parsePosition(parts[1], words.size) ?: continue
        val j = parsePosition(parts[2], words.size) ?: continue
        if (command == "swap") {
            swap(words, i - 1, j - 1)
            swaps++
        } else {
            swaps += reverse(words, minOf(i, j) - 1, maxOf(i, j) - 1)
        }
        println(words.joinToString(" "))
    }
    println("swaps: $swaps")
}
