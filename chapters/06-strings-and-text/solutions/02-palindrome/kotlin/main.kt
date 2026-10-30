// codePoints() walks whole code points. Reversing the String itself would work on
// UTF-16 Chars and tear every emoji in half.
fun isPalindrome(text: String): Boolean {
    val kept =
        text
            .codePoints()
            .toArray()
            .filter { it > 0x7F || Character.isLetterOrDigit(it) }
            .map { if (it > 0x7F) it else Character.toLowerCase(it) }
    return kept == kept.reversed()
}

fun main() {
    while (true) {
        val text = readlnOrNull()?.trim() ?: break
        if (text.isEmpty()) continue
        println("$text: ${if (isPalindrome(text)) "palindrome" else "not a palindrome"}")
    }
}
