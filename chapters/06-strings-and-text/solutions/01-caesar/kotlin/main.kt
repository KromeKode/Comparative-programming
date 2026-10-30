// Char arithmetic: 'a' + 3 is 'd', and 'd' - 'a' is 3.
fun shiftLetter(c: Char, shift: Int): Char =
    when (c) {
        in 'a'..'z' -> 'a' + (c - 'a' + shift).mod(26)
        in 'A'..'Z' -> 'A' + (c - 'A' + shift).mod(26)
        else -> c
    }

fun main() {
    while (true) {
        val line = readlnOrNull()?.trim() ?: break
        if (line.isEmpty()) continue
        val number = line.substringBefore(' ')
        val text = line.substringAfter(' ', "")
        val shift = number.toInt()
        // map works Char by Char. That is safe here: no half of an emoji is a letter.
        println(text.map { shiftLetter(it, shift) }.joinToString(""))
    }
}
