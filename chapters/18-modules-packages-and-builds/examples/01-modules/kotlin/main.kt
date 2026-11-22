// main.kt has no package statement, so its names are in the default package.
// It imports the names it uses from textstats; kotlinc compiles both files.
import textstats.VERSION
import textstats.longestWord
import textstats.wordCount

fun main() {
    val text = generateSequence(::readlnOrNull).joinToString("\n")
    println("textstats $VERSION")
    println("words: ${wordCount(text)}")
    println("longest: ${longestWord(text)}")
}
