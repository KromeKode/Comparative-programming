val memo = mutableMapOf<Int, Long>()

fun fib(n: Int): Long {
    if (n < 2) return n.toLong()
    // getOrPut runs the block only when n is not in the map yet. It allows the block
    // to add other keys, as the recursion does; Java's computeIfAbsent does not.
    return memo.getOrPut(n) { fib(n - 1) + fib(n - 2) }
}

fun main() {
    while (true) {
        val line = readlnOrNull() ?: break
        val text = line.trim()
        if (text.isEmpty()) continue
        val n = text.toInt()
        println("fib($n) = ${fib(n)}")
    }
}
