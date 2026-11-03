// Extension functions add the adapters to every Sequence<Long>, so they chain
// like the built-in ones. Each builds a lazy sequence that pulls one item at a time.
fun Sequence<Long>.windows(k: Int): Sequence<List<Long>> = sequence {
    val window = ArrayDeque<Long>()
    for (item in this@windows) {
        window.addLast(item)
        if (window.size > k) window.removeFirst() // the oldest item leaves
        if (window.size == k) yield(window.toList()) // a copy, since window changes
    }
}

fun Sequence<Long>.chunks(k: Int): Sequence<List<Long>> = sequence {
    var chunk = mutableListOf<Long>()
    for (item in this@chunks) {
        chunk.add(item)
        if (chunk.size == k) {
            yield(chunk)
            chunk = mutableListOf()
        }
    }
    if (chunk.isNotEmpty()) yield(chunk)
}

fun Sequence<Long>.runs(): Sequence<List<Long>> = sequence {
    var run = mutableListOf<Long>()
    for (item in this@runs) {
        if (run.isNotEmpty() && item != run.first()) {
            yield(run)
            run = mutableListOf()
        }
        run.add(item)
    }
    if (run.isNotEmpty()) yield(run)
}

fun show(groups: Sequence<List<Long>>): String =
    groups.joinToString(" ") { group -> group.joinToString(" ", "[", "]") }

fun answer(words: List<String>): String {
    val text =
        if (words[0] == "runs") {
            show(words.asSequence().drop(1).map(String::toLong).runs())
        } else {
            val k = words[1].toInt()
            val numbers = words.asSequence().drop(2).map(String::toLong)
            when (words[0]) {
                "windows" -> show(numbers.windows(k))
                "chunks" -> show(numbers.chunks(k))
                else -> numbers.windows(k).map { it.sum() }.joinToString(" ")
            }
        }
    return text.ifEmpty { "(none)" }
}

fun main() {
    for (line in generateSequence(::readlnOrNull)) {
        val words = line.split(" ").filter { it.isNotEmpty() }
        if (words.isNotEmpty()) println(answer(words))
    }
}
