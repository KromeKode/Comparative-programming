val table =
    listOf(
        1000 to "M",
        900 to "CM",
        500 to "D",
        400 to "CD",
        100 to "C",
        90 to "XC",
        50 to "L",
        40 to "XL",
        10 to "X",
        9 to "IX",
        5 to "V",
        4 to "IV",
        1 to "I",
    )

fun toRoman(number: Int): String = buildString {
    var n = number // parameters are read-only, so copy into a var
    for ((value, symbol) in table) {
        while (n >= value) {
            append(symbol) // take the biggest value that still fits
            n -= value
        }
    }
}

fun letterValue(letter: Char): Int =
    when (letter) {
        'I' -> 1
        'V' -> 5
        'X' -> 10
        'L' -> 50
        'C' -> 100
        'D' -> 500
        'M' -> 1000
        else -> throw IllegalArgumentException("not a Roman numeral letter: $letter")
    }

fun fromRoman(text: String): Int {
    var total = 0
    for (i in text.indices) {
        val value = letterValue(text[i])
        // getOrNull gives null past the end, so the bounds check is built in.
        val next = text.getOrNull(i + 1)
        // A smaller letter before a bigger one is subtracted: IV is 5 - 1.
        if (next != null && value < letterValue(next)) {
            total -= value
        } else {
            total += value
        }
    }
    return total
}

fun main() {
    while (true) {
        val line = readlnOrNull() ?: break
        val text = line.trim()
        if (text.isEmpty()) continue
        val n = text.toIntOrNull()
        if (n != null) {
            println("$n -> ${toRoman(n)}")
        } else {
            println("$text -> ${fromRoman(text)}")
        }
    }
}
