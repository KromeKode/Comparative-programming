class LineError(message: String) : Exception(message)

data class Version(val major: Int, val minor: Int, val patch: Int) : Comparable<Version> {
    // Part by part, as numbers: major first, then minor, then patch.
    override fun compareTo(other: Version) =
        compareValuesBy(this, other, { it.major }, { it.minor }, { it.patch })

    override fun toString() = "$major.$minor.$patch"
}

data class Interval<T : Comparable<T>>(val low: T, val high: T) {
    override fun toString() = "$low..$high"
}

// Sort by the low ends, then join each interval to the last one if they overlap.
fun <T : Comparable<T>> merge(intervals: List<Interval<T>>): List<Interval<T>> {
    val merged = mutableListOf<Interval<T>>()
    for (interval in intervals.sortedBy { it.low }) {
        val last = merged.lastOrNull()
        if (last != null && interval.low <= last.high) {
            merged[merged.lastIndex] = Interval(last.low, maxOf(last.high, interval.high))
        } else {
            merged.add(interval)
        }
    }
    return merged
}

val whole = Regex("-?[0-9]+")
val digits = Regex("[0-9]+")
val letters = Regex("[a-z]+")

// toLongOrNull would also take "+5", so check the form first.
fun parseInt(text: String): Long? = if (whole.matches(text)) text.toLongOrNull() else null

fun parseWord(text: String): String? = if (letters.matches(text)) text else null

fun parseVersion(text: String): Version? {
    val parts = text.split(".")
    if (parts.size != 3 || !parts.all { digits.matches(it) }) return null
    val (major, minor, patch) = parts.map { it.toIntOrNull() ?: return null }
    return Version(major, minor, patch)
}

// Parse every interval on a line, merge them, and format the result.
fun <T : Comparable<T>> run(kind: String, parse: (String) -> T?, words: List<String>): String {
    val intervals = words.map { word ->
        val ends = word.split("..")
        if (ends.size != 2) throw LineError("bad interval \"$word\"")
        val low = parse(ends[0]) ?: throw LineError("bad $kind \"${ends[0]}\"")
        val high = parse(ends[1]) ?: throw LineError("bad $kind \"${ends[1]}\"")
        if (high < low) throw LineError("$word is backwards")
        Interval(low, high)
    }
    return (listOf(kind) + merge(intervals)).joinToString(" ")
}

fun main() {
    for (line in generateSequence(::readlnOrNull)) {
        val words = line.split(" ").filter { it.isNotEmpty() }
        if (words.isEmpty()) continue
        val (kind, rest) = words.first() to words.drop(1)
        try {
            val text =
                when (kind) {
                    "int" -> run(kind, ::parseInt, rest)
                    "word" -> run(kind, ::parseWord, rest)
                    "version" -> run(kind, ::parseVersion, rest)
                    else -> throw LineError("unknown kind \"$kind\"")
                }
            println(text)
        } catch (error: LineError) {
            println("error: ${error.message}")
        }
    }
}
