// Durations such as 1h30m: parseDuration reads one, and formatDuration writes one.

private const val UNITS = "dhms" // largest first, which is the order they come in
private val SECONDS = longArrayOf(86400, 3600, 60, 1) // in each of the units

/**
 * The number of seconds that [text] says, such as 5400 for "1h30m". Throws an
 * [IllegalArgumentException] if [text] isn't a duration.
 */
fun parseDuration(text: String): Long {
    require(text.isNotEmpty()) { "a duration can't be empty" }
    var seconds = 0L
    var nextUnit = 0 // each part's unit comes after the one before it in UNITS
    var i = 0
    while (i < text.length) {
        val start = i
        while (i < text.length && text[i] in '0'..'9') {
            i++
        }
        require(i > start) { "\"$text\" has a part with no number" }
        require(i < text.length) { "\"$text\" ends with a number that has no unit" }
        val number = text.substring(start, i).toLong()
        val unit = UNITS.indexOf(text[i])
        require(unit != -1) { "'${text[i]}' isn't a unit" }
        require(unit >= nextUnit) { "\"$text\" has a unit out of order, or twice" }
        // These throw an ArithmeticException if the duration is too long to count.
        seconds = Math.addExact(seconds, Math.multiplyExact(number, SECONDS[unit]))
        nextUnit = unit + 1
        i++
    }
    return seconds
}

/** [seconds] as a duration, such as "1h30m" for 5400, leaving out the units that are 0. */
fun formatDuration(seconds: Long): String {
    if (seconds == 0L) return "0s"
    var left = seconds
    return buildString {
        for (unit in UNITS.indices) {
            val amount = left / SECONDS[unit]
            left %= SECONDS[unit]
            if (amount > 0) append("$amount${UNITS[unit]}")
        }
    }
}
