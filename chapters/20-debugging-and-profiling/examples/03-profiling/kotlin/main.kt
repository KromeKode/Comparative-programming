// Counts the words in its input, and prints the ten most common. It is slow on a long
// text, and section 20.7 runs it under a profiler to find out why.

class Count(val word: String, var n: Int)

/** Gives the words of the text: runs of the letters a to z, in lowercase. */
fun splitWords(text: String): List<String> =
    text.lowercase().split(Regex("[^a-z]+")).filter { it.isNotEmpty() }

/** Gives the index of the word's count, or -1 if it has none. */
fun find(counts: List<Count>, word: String): Int {
    for (i in counts.indices) {
        if (counts[i].word == word) return i
    }
    return -1
}

fun countWords(words: List<String>): List<Count> {
    val counts = mutableListOf<Count>()
    for (word in words) {
        val i = find(counts, word)
        if (i == -1) counts += Count(word, 1) else counts[i].n++
    }
    return counts
}

fun main() {
    val counts = countWords(splitWords(System.`in`.bufferedReader().readText()))
    val top = counts.sortedWith(compareBy({ -it.n }, { it.word })).take(10)
    for (count in top) println("${count.n} ${count.word}")
}
