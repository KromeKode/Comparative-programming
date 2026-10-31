fun main() {
    val words =
        System.`in`.bufferedReader().readText().split(Regex("\\s+")).filter { it.isNotEmpty() }
    // groupBy builds a LinkedHashMap, so the groups come out in the order their
    // first words appeared.
    val groups =
        words
            .map { it.lowercase() }
            .groupBy { word -> word.toCharArray().sorted().joinToString("") }
    for (group in groups.values) {
        println(group.joinToString(" "))
    }
}
