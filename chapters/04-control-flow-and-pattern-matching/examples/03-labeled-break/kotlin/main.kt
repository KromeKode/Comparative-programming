val grid = listOf(listOf(3, 8, 1), listOf(9, 4, 7), listOf(2, 6, 5))

fun main() {
    targets@ for (target in listOf(7, 10)) {
        var checked = 0
        for ((r, row) in grid.withIndex()) {
            for ((c, value) in row.withIndex()) {
                checked++
                if (value == target) {
                    println("$target: row $r, column $c (checked $checked cells)")
                    continue@targets // leaves both inner loops at once
                }
            }
        }
        println("$target: not found (checked $checked cells)")
    }
}
