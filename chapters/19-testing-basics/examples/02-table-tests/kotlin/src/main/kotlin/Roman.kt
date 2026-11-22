private val numerals =
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

/** [n] in Roman numerals. Throws an IllegalArgumentException if [n] is outside 1 to 3999. */
fun toRoman(n: Int): String {
    require(n in 1..3999) { "$n is out of range" }
    var rest = n
    return buildString {
        for ((value, letters) in numerals) {
            while (rest >= value) {
                append(letters)
                rest -= value
            }
        }
    }
}
