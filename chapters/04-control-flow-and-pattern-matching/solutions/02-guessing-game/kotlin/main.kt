fun guessesText(count: Int) = if (count == 1) "1 guess" else "$count guesses"

fun main() {
    val secret = readln().trim().toInt()

    var guesses = 0
    while (true) {
        val line = readlnOrNull() ?: break
        val text = line.trim()
        if (text.isEmpty()) continue // a blank line
        val guess = text.toIntOrNull() // null when the text is not a whole number
        if (guess == null) {
            println("$text: not a number")
            continue
        }
        guesses++
        // A when with no subject is a cleaner else-if chain.
        when {
            guess < secret -> println("$guess: too low")
            guess > secret -> println("$guess: too high")
            else -> {
                println("$guess: correct in ${guessesText(guesses)}")
                return
            }
        }
    }
    println("gave up after ${guessesText(guesses)}; the number was $secret")
}
