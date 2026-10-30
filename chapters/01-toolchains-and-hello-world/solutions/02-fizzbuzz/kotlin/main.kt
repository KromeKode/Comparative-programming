import kotlin.system.exitProcess

fun main() {
    val n = (readlnOrNull() ?: "").trim().toIntOrNull()
    if (n == null) {
        System.err.println("expected a whole number")
        exitProcess(1) // returns Nothing, so below this line Kotlin knows n is an Int
    }
    for (i in 1..n) {
        if (i % 15 == 0) {
            println("FizzBuzz")
        } else if (i % 3 == 0) {
            println("Fizz")
        } else if (i % 5 == 0) {
            println("Buzz")
        } else {
            println(i)
        }
    }
}
