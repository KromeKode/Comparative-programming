fun yesNo(condition: Boolean) = if (condition) "yes" else "no"

fun main() {
    val text = "  The quick  brown fox  "

    val trimmed = text.trim()
    println("trimmed: [$trimmed]")
    println("words: ${trimmed.split(Regex("\\s+")).joinToString("|")}")
    println("upper: ${trimmed.uppercase()}")
    println("contains \"quick\": ${yesNo("quick" in trimmed)}")
    println("index of \"brown\": ${trimmed.indexOf("brown")}") // -1 if missing
    println("replaced: ${trimmed.replace("quick", "slow")}")
    println("starts with \"The\": ${yesNo(trimmed.startsWith("The"))}")

    val fruits = mutableListOf("banana", "apple", "Cherry")
    fruits.sort() // compares UTF-16 code units
    println("sorted: ${fruits.joinToString(" ")}")
    fruits.sortWith(String.CASE_INSENSITIVE_ORDER)
    println("sorted ignoring case: ${fruits.joinToString(" ")}")
}
