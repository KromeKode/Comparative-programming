// Copies stdin to stdout, with each date in it written as YYYY-MM-DD.
val MONTHS =
    "January February March April May June July August September October November December"
        .split(" ")

// Each month's name, or its first three letters: "Jan(?:uary)?", and so on.
val MONTH =
    MONTHS.joinToString("|") { if (it.length > 3) "${it.take(3)}(?:${it.drop(3)})?" else it }
const val D = "([0-9]{1,2})"
const val M = "([0-9]{1,2})"
const val Y = "([0-9]{4})"
// In Java 21, which Kotlin runs on here, \b looks at ASCII letters, digits, and _ alone,
// as \w does, and \d is only [0-9].
val DATE = Regex("""\b(?:$D/$M/$Y|($MONTH) $D, $Y|$D ($MONTH) $Y)\b""")

fun daysIn(month: Int, year: Int) =
    when {
        month == 2 && year % 4 == 0 && (year % 100 != 0 || year % 400 == 0) -> 29
        month == 2 -> 28
        month in listOf(4, 6, 9, 11) -> 30
        else -> 31
    }

fun monthOf(name: String) = MONTHS.indexOfFirst { it.startsWith(name) } + 1

/** The date as YYYY-MM-DD, or the text as it was if it isn't a real date. */
fun rewrite(match: MatchResult): String {
    // Only one of the three forms matched, so the groups of the other two are "".
    val g = match.groupValues
    val (day, month, year) =
        when {
            g[1] != "" -> Triple(g[1].toInt(), g[2].toInt(), g[3].toInt())
            g[4] != "" -> Triple(g[5].toInt(), monthOf(g[4]), g[6].toInt())
            else -> Triple(g[7].toInt(), monthOf(g[8]), g[9].toInt())
        }
    if (month !in 1..12 || day !in 1..daysIn(month, year)) {
        return match.value // not a real date, so left as it was
    }
    // Not "%04d".format, which writes digits as the locale has them.
    fun pad(n: Int, width: Int) = n.toString().padStart(width, '0')
    return "${pad(year, 4)}-${pad(month, 2)}-${pad(day, 2)}"
}

fun main() {
    val text = System.`in`.readBytes().decodeToString()
    System.out.write(DATE.replace(text, ::rewrite).encodeToByteArray())
    System.out.flush()
}
