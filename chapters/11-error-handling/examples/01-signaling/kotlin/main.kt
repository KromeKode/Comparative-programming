fun main() {
    val inputs = listOf("42", "4x2", "", "99999999999999999999")

    // toInt throws a NumberFormatException when it cannot parse.
    println("toInt:")
    for (text in inputs) {
        try {
            println("  \"$text\" -> ${text.toInt()}")
        } catch (error: NumberFormatException) {
            println("  \"$text\" -> NumberFormatException: ${error.message}")
        }
    }

    // toIntOrNull returns null instead, and the type Int? makes you check.
    println("toIntOrNull:")
    for (text in inputs) {
        val value = text.toIntOrNull()
        println("  \"$text\" -> ${value ?: "null, so use a default: 0"}")
    }
}
