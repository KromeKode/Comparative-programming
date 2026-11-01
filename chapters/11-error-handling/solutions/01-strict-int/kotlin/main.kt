// A sealed class: the when in message must cover every subclass.
sealed class ParseError : Exception() {
    override val message: String
        get() =
            when (this) {
                is Empty -> "empty input"
                is NoDigits -> "no digits after the sign"
                is InvalidCharacter -> "invalid character '$character' at position $position"
                is TooLarge -> "too large, the maximum is ${Int.MAX_VALUE}"
                is TooSmall -> "too small, the minimum is ${Int.MIN_VALUE}"
            }
}

class Empty : ParseError()

class NoDigits : ParseError()

class InvalidCharacter(val character: Char, val position: Int) : ParseError()

class TooLarge : ParseError()

class TooSmall : ParseError()

fun parseInt32(text: String): Int {
    if (text.isEmpty()) throw Empty()
    val negative = text[0] == '-'
    val start = if (text[0] == '+' || text[0] == '-') 1 else 0
    if (start == text.length) throw NoDigits()
    // Check every character first, so a bad one is reported even when the
    // digits before it are already too large.
    for (i in start until text.length) {
        if (text[i] !in '0'..'9') throw InvalidCharacter(text[i], i + 1)
    }
    // Stop growing once the number is past every limit, so it cannot
    // overflow however many digits there are.
    val limit = Int.MAX_VALUE.toLong() + 2
    var magnitude = 0L
    for (i in start until text.length) {
        magnitude = minOf(magnitude * 10 + (text[i] - '0'), limit)
    }
    val value = if (negative) -magnitude else magnitude
    if (value > Int.MAX_VALUE) throw TooLarge()
    if (value < Int.MIN_VALUE) throw TooSmall()
    return value.toInt()
}

fun main() {
    while (true) {
        val line = readlnOrNull() ?: break
        try {
            println("\"$line\" -> ${parseInt32(line)}")
        } catch (e: ParseError) {
            println("\"$line\" -> error: ${e.message}")
        }
    }
}
