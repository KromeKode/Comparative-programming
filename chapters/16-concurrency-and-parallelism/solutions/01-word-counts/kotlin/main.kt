import java.util.concurrent.Callable
import java.util.concurrent.Executors

fun plural(count: Int, noun: String) = if (count == 1) "$count $noun" else "$count ${noun}s"

val word = Regex("[A-Za-z]+")

// Runs on a thread of the pool. It reads only its own lines, which nothing
// changes while it runs, and returns a map of its own.
fun countWords(lines: List<String>): Map<String, Int> {
    val counts = mutableMapOf<String, Int>()
    for (line in lines) {
        for (match in word.findAll(line)) {
            counts.merge(match.value.lowercase(), 1, Int::plus)
        }
    }
    return counts
}

fun main() {
    val lines = generateSequence(::readlnOrNull).toList()
    val first = lines.firstOrNull()?.trim()?.split(Regex("\\s+")) ?: emptyList()
    if (first.size != 2 || !first.all { it.isNotEmpty() && it.all { c -> c in '0'..'9' } }) {
        println("error: the first line must be two numbers, workers and top")
        return
    }
    val (workers, top) = first.map(String::toInt)
    if (workers !in 1..16) {
        println("error: workers must be from 1 to 16")
        return
    }

    // Each worker gets text.size / workers lines, and the first few get one
    // more, so that every line is counted once.
    val text = lines.drop(1)
    val chunks = mutableListOf<List<String>>()
    var start = 0
    for (i in 0 until workers) {
        val size = text.size / workers + if (i < text.size % workers) 1 else 0
        chunks.add(text.subList(start, start + size))
        start += size
    }

    // invokeAll runs every task on the pool's threads, waits for all of them,
    // and returns their Futures in the order the tasks were given.
    val counts =
        Executors.newFixedThreadPool(workers).use { pool ->
            pool.invokeAll(chunks.map { chunk -> Callable { countWords(chunk) } }).map { it.get() }
        }

    println("${plural(workers, "worker")}, ${plural(text.size, "line")}")
    val total = mutableMapOf<String, Int>()
    for ((i, chunk) in chunks.withIndex()) {
        val words = counts[i].values.sum()
        println("worker $i: ${plural(chunk.size, "line")}, ${plural(words, "word")}")
        counts[i].forEach { (word, count) -> total.merge(word, count, Int::plus) }
    }
    println("${plural(total.values.sum(), "word")}, ${total.size} different")

    val sorted =
        total.entries.sortedWith(
            compareByDescending<Map.Entry<String, Int>> { it.value }.thenBy { it.key }
        )
    for ((word, count) in sorted.take(top)) println("$word: $count")
}
