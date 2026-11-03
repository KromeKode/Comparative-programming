data class Score(val name: String, val points: Int)

val scores =
    listOf(
        Score("ada", 91),
        Score("bob", 78),
        Score("cy", 85),
        Score("dee", 62),
        Score("eve", 99),
        Score("finn", 54),
    )

fun yesNo(value: Boolean) = if (value) "yes" else "no"

fun main() {
    // List operations are eager: filter builds a new list right away.
    val passes = scores.filter { it.points >= 70 }
    println("passed: ${passes.joinToString(", ") { it.name }}")
    println("total of the passes: ${passes.sumOf { it.points }}")

    val best = scores.maxBy { it.points }
    println("best: ${best.name} with ${best.points}")
    println("everyone passed: ${yesNo(scores.all { it.points >= 70 })}")
    println("anyone above 95: ${yesNo(scores.any { it.points > 95 })}")

    // withIndex pairs each item with its index, and (i, score) takes it apart.
    val top =
        scores
            .sortedByDescending { it.points }
            .take(3)
            .withIndex()
            .joinToString(", ") { (i, score) ->
                "${i + 1}. ${score.name} ${score.points}"
            }
    println("top three: $top")

    // flatMap turns each name into its letters, and a sorted set keeps one of each.
    val letters = scores.flatMap { it.name.toList() }.toSortedSet()
    println("letters used: ${letters.joinToString(" ")}")
}
