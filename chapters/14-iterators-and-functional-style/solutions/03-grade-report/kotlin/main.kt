data class Grade(val name: String, val subject: String, val score: Int)

fun report(grades: List<Grade>): List<String> {
    val lines = mutableListOf("subjects:")
    // groupBy makes a map from each key to the list of items with that key.
    for ((subject, group) in grades.groupBy { it.subject }.toSortedMap()) {
        val best = group.maxOf { it.score }
        val names = group.filter { it.score == best }.map { it.name }.sorted()
        val average = group.sumOf { it.score } / group.size
        lines += "  $subject: average $average, best $best (${names.joinToString(", ")})"
    }

    val students = grades.groupBy { it.name }
    val ranked =
        students
            .map { (name, group) -> name to group.sumOf { it.score } }
            .sortedWith(compareByDescending<Pair<String, Int>> { it.second }.thenBy { it.first })
    lines += "ranking:"
    lines += ranked.mapIndexed { i, (name, total) -> "  ${i + 1}. $name $total" }

    val passed = students.filterValues { group -> group.all { it.score >= 50 } }.keys.sorted()
    lines += "passed everything: ${passed.joinToString(", ").ifEmpty { "nobody" }}"
    return lines
}

fun main() {
    val grades =
        generateSequence(::readlnOrNull)
            .map { it.trim().split(Regex("\\s+")) }
            .filter { it.size >= 3 }
            .map { (name, subject, score) -> Grade(name, subject, score.toInt()) }
            .toList()
    println(report(grades).joinToString("\n"))
}
