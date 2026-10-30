import java.text.BreakIterator

fun main() {
    // A Kotlin String is a sequence of UTF-16 code units. \u takes exactly four hex
    // digits, so U+1F44D and U+1F3FD are written as their two UTF-16 halves.
    val s = "Zo\u00eb \uD83D\uDC4D\uD83C\uDFFD"

    println("text: $s")
    println("built-in length: ${s.length} (UTF-16 code units)")
    println("UTF-8 bytes: ${s.toByteArray().size}") // toByteArray() encodes as UTF-8
    println("UTF-16 code units: ${s.length}")
    println("code points: ${s.codePointCount(0, s.length)}")

    // Java's BreakIterator finds grapheme clusters, the characters a reader sees.
    val boundaries = BreakIterator.getCharacterInstance()
    boundaries.setText(s)
    var graphemes = 0
    while (boundaries.next() != BreakIterator.DONE) graphemes++
    println("graphemes: $graphemes")
}
