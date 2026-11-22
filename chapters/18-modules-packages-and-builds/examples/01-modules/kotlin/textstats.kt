// The package statement puts this file's names in the package textstats.
// Top-level names are public by default; private ones can be used only in this
// file, and internal ones only in the same module, the set of files that are
// compiled together.
package textstats

const val VERSION = "1.0"

private val word = Regex("[A-Za-z]+")

// Private to this file.
private fun words(text: String): Sequence<String> = word.findAll(text).map { it.value }

/** The number of words in [text]. A word is a run of ASCII letters. */
fun wordCount(text: String): Int = words(text).count()

/** The first of the longest words in [text]. */
fun longestWord(text: String): String =
    words(text).fold("") { best, word -> if (word.length > best.length) word else best }
