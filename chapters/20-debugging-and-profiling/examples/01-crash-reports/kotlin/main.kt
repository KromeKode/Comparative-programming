// Counts the letters in its input, and draws a bar for each one. It expects lowercase
// text, and doesn't check: section 20.3 gives it capitals, to see what happens.
fun countLetters(text: String): IntArray {
    val counts = IntArray(26)
    for (c in text) {
        if (c.isLetter()) counts[c - 'a']++
    }
    return counts
}

fun main() {
    val counts = countLetters(System.`in`.bufferedReader().readText())
    for (i in counts.indices) {
        if (counts[i] > 0) println("${'a' + i} ${counts[i]} ${"#".repeat(counts[i])}")
    }
}
