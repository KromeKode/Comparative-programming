var calls = 0 // counts how many times the right side ran

fun bigRatio(x: Int): Boolean {
    calls++
    return 10 / x > 1 // dividing by zero here would throw
}

fun displayName(name: String): String {
    // Conditions must be Boolean, so test isEmpty() explicitly.
    return if (name.isEmpty()) "stranger" else name
}

fun main() {
    for (x in listOf(0, 5)) {
        // && stops as soon as the left side is false, so bigRatio never sees 0.
        val ok = x != 0 && bigRatio(x)
        println("x = $x: ${if (ok) "yes" else "no"}")
    }
    println("right side ran $calls of 2 times")
    println("name \"\": ${displayName("")}")
    println("name \"Ada\": ${displayName("Ada")}")
}
