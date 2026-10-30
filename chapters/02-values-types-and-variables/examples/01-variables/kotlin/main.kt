const val MAX_PLAYERS = 4 // compile-time constant: allowed only at top level or in an object

fun main() {
    var count = 0 // var: can be reassigned; the type (Int) is inferred
    val birthYear = 1815 // val: assigned exactly once
    val pi = 3.14159
    val ready: Boolean = true // types can also be written out

    count += 3

    println("count = $count")
    println("birth year = $birthYear")
    println("pi = $pi")
    println("max players = $MAX_PLAYERS")
    println("ready = $ready")
}
