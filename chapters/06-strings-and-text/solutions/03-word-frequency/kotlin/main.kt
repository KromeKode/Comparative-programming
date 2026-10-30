// A word is a run of ASCII letters and non-ASCII characters. Checking Char by Char is
// safe here: both UTF-16 halves of an emoji are non-ASCII, so they stay in the word.
fun isWordChar(c: Char) = c in 'a'..'z' || c in 'A'..'Z' || c.code > 0x7F

fun words(text: String): List<String> {
    val found = mutableListOf<String>()
    val current = StringBuilder()
    for (c in "$text ") { // the extra space ends the last word
        if (isWordChar(c)) {
            current.append(if (c in 'A'..'Z') c.lowercaseChar() else c)
        } else if (current.isNotEmpty()) {
            found.add(current.toString())
            current.clear()
        }
    }
    return found
}

fun main() {
    val text = System.`in`.bufferedReader().readText() // UTF-8 by default
    val counts = words(text).groupingBy { it }.eachCount()

    // Most frequent first; ties in UTF-16 code unit order, which is how compareTo works.
    val sorted = counts.entries.sortedWith(compareBy({ -it.value }, { it.key }))
    for ((word, count) in sorted) {
        println("$count $word")
    }
}
