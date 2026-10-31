// Returns the pair with the smallest j, and for that j the smallest i.
fun twoSum(target: Long, numbers: List<Long>): Pair<Int, Int>? {
    // Each number seen so far, mapped to the first index where it appeared.
    val firstIndex = HashMap<Long, Int>()
    for ((j, n) in numbers.withIndex()) {
        val i = firstIndex[target - n]
        if (i != null) return i to j
        firstIndex.putIfAbsent(n, j)
    }
    return null
}

fun main() {
    while (true) {
        val line = readlnOrNull() ?: break
        if (':' !in line) continue // a blank line
        val target = line.substringBefore(':').trim().toLong()
        val numbers =
            line
                .substringAfter(':')
                .split(' ')
                .filter { it.isNotBlank() }
                .map { it.trim().toLong() }
        val pair = twoSum(target, numbers)
        println(if (pair == null) "none" else "${pair.first} ${pair.second}")
    }
}
