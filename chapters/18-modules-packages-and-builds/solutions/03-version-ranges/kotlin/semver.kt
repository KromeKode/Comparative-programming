// Versions and ranges as npm writes them: 1.2.3-beta.1, ^1.2, ~1.2.3, >=1 <2, 1.x || 3.
//
// Everything marked private is visible only in this file. main.kt can use
// Version, parseVersion, compare, Range, and Range.parse.
package semver

private const val NUMBER = "0|[1-9][0-9]{0,14}"
private const val IDENTIFIER = "(?:0|[1-9][0-9]*|[0-9]*[A-Za-z-][0-9A-Za-z-]*)"
private const val PRE = "$IDENTIFIER(?:\\.$IDENTIFIER)*"
private val versionPattern = Regex("($NUMBER)\\.($NUMBER)\\.($NUMBER)(?:-($PRE))?")
private const val PART = "(?:$NUMBER|[xX*])"

// A partial version, 1 or 1.2 or 1.2.3, with x, X, or * for any part. A pre-release
// can follow only three numbers, which expand checks.
private val partialPattern = Regex("($PART)(?:\\.($PART)(?:\\.($PART)(?:-($PRE))?)?)?")
private val operatorPattern = Regex("^(<=|>=|<|>|=|\\^|~)?")
private val digits = Regex("[0-9]+")

private fun isWild(part: String) = part == "x" || part == "X" || part == "*"

data class Version(
    val major: Long,
    val minor: Long,
    val patch: Long,
    /** The pre-release's identifiers, if it has one. */
    val pre: List<String> = emptyList(),
) {
    // internal: visible in this module, which is everything kotlinc compiles at once.
    // main.kt could call it too, but a program that uses semver as a library couldn't.
    internal fun sameCore(other: Version) =
        major == other.major && minor == other.minor && patch == other.patch

    override fun toString() =
        "$major.$minor.$patch" + if (pre.isEmpty()) "" else "-" + pre.joinToString(".")
}

/** The version that [text] is, or null if it isn't one. */
fun parseVersion(text: String): Version? {
    val match = versionPattern.matchEntire(text) ?: return null
    val (major, minor, patch, pre) = match.destructured
    return Version(major.toLong(), minor.toLong(), patch.toLong(), splitPre(pre))
}

private fun splitPre(pre: String) = if (pre.isEmpty()) emptyList() else pre.split(".")

/**
 * Numbers compare as numbers, and come before other identifiers, which compare
 * character by character. Numbers have no 0 in front, so a longer one is larger.
 */
private fun compareIdentifiers(a: String, b: String): Int {
    val aNumber = digits.matches(a)
    val bNumber = digits.matches(b)
    return when {
        aNumber && bNumber -> compareValuesBy(a, b, { it.length }, { it })
        aNumber -> -1
        bNumber -> 1
        else -> a.compareTo(b).coerceIn(-1, 1)
    }
}

/** Negative if [a] comes before [b], 0 if they are equal, and positive if after. */
fun compare(a: Version, b: Version): Int {
    val core = compareValuesBy(a, b, { it.major }, { it.minor }, { it.patch })
    if (core != 0) return core
    // A version without a pre-release comes after the same version with one.
    if (a.pre.isEmpty() || b.pre.isEmpty()) return b.pre.size.compareTo(a.pre.size)
    for ((x, y) in a.pre.zip(b.pre)) {
        val order = compareIdentifiers(x, y)
        if (order != 0) return order
    }
    return a.pre.size.compareTo(b.pre.size)
}

private enum class Op { EQ, LT, LE, GT, GE }

/** A simple comparator: an operator and a full version. */
private typealias Simple = Pair<Op, Version>

/** The simple comparators that a comparator stands for, or null if it isn't one. */
private fun expand(token: String): List<Simple>? {
    val op = operatorPattern.find(token)!!.value
    val match = partialPattern.matchEntire(token.substring(op.length)) ?: return null
    // An optional group that didn't match is "" in groupValues.
    val parts = match.groupValues.subList(1, 4)
    val pre = match.groupValues[4]
    // known counts the numbers, which must come before any wildcards.
    var known = 0
    while (known < 3 && parts[known].isNotEmpty() && !isWild(parts[known])) known++
    if (parts.drop(known).any { it.isNotEmpty() && !isWild(it) }) return null
    // A pre-release can follow only three numbers.
    if (pre.isNotEmpty() && known < 3) return null
    val nums = List(3) { if (it < known) parts[it].toLong() else 0L }
    val low = Version(nums[0], nums[1], nums[2], splitPre(pre))

    // The first version after all those whose parts up to level match: the -0
    // pre-release of the next major, minor, or patch.
    fun after(level: Int) = when (level) {
        0 -> Version(nums[0] + 1, 0, 0, listOf("0"))
        1 -> Version(nums[0], nums[1] + 1, 0, listOf("0"))
        else -> Version(nums[0], nums[1], nums[2] + 1, listOf("0"))
    }
    if (known == 0) {
        // Any version: nothing is outside it, and nothing is above or below it.
        val first = Version(0, 0, 0, listOf("0"))
        return if (op == "<" || op == ">") listOf(Op.LT to first) else listOf()
    }
    val full = known == 3
    val next = after(known - 1)
    return when (op) {
        "", "=" -> if (full) listOf(Op.EQ to low) else listOf(Op.GE to low, Op.LT to next)
        ">" -> listOf(if (full) Op.GT to low else Op.GE to next.copy(pre = emptyList()))
        ">=" -> listOf(Op.GE to low)
        "<" -> listOf(Op.LT to if (full) low else low.copy(pre = listOf("0")))
        "<=" -> listOf(if (full) Op.LE to low else Op.LT to next)
        "~" -> listOf(Op.GE to low, Op.LT to after(if (known == 1) 0 else 1))
        else -> {
            // ^ allows changes that don't change the first part that isn't 0.
            val first = nums.take(known).indexOfFirst { it != 0L }
            listOf(Op.GE to low, Op.LT to after(if (first >= 0) first else known - 1))
        }
    }
}

private fun holds(version: Version, simple: Simple): Boolean {
    val (op, bound) = simple
    val order = compare(version, bound)
    return when (op) {
        Op.EQ -> order == 0
        Op.LT -> order < 0
        Op.LE -> order <= 0
        Op.GT -> order > 0
        Op.GE -> order >= 0
    }
}

/**
 * Sets of simple comparators joined by ||. A version is in the range if it passes
 * every comparator of some set. The constructor is private, so only Range.parse can
 * make a Range.
 */
class Range private constructor(private val sets: List<List<Simple>>) {
    fun allows(version: Version) = sets.any { simples ->
        simples.all { holds(version, it) } &&
            // A pre-release is in only if a comparator of the set names a pre-release
            // of the same major, minor, and patch.
            (version.pre.isEmpty() ||
                simples.any { (_, bound) -> bound.pre.isNotEmpty() && bound.sameCore(version) })
    }

    /** The highest of [versions] that the range allows, or null if it allows none. */
    fun highest(versions: List<Version>): Version? =
        versions.filter(::allows).fold(null as Version?) { best, version ->
            if (best == null || compare(version, best) > 0) version else best
        }

    // A companion object holds what belongs to the class rather than to one Range,
    // called as Range.parse. Being inside the class, it can call the private
    // constructor.
    companion object {
        /** The range that [text] is. Throws IllegalArgumentException, saying why, if it isn't. */
        fun parse(text: String): Range = Range(
            text.split("||").map { side ->
                val tokens = side.split(Regex("\\s+")).filter { it.isNotEmpty() }
                require(tokens.isNotEmpty()) { "each side of || needs a comparator" }
                tokens.flatMap { requireNotNull(expand(it)) { "$it isn't a comparator" } }
            },
        )
    }
}
