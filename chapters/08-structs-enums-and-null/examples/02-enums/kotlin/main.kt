// An enum class can carry properties, set by its constructor.
enum class Light(val seconds: Int) {
    RED(30),
    GREEN(25),
    YELLOW(5);

    val next: Light
        get() = entries[(ordinal + 1) % entries.size]

    // A when expression must cover every value, or it does not compile.
    val meaning: String
        get() =
            when (this) {
                RED -> "stop"
                GREEN -> "go"
                YELLOW -> "slow down"
            }

    override fun toString() = name.lowercase()
}

// find returns null when nothing matches.
fun parseLight(text: String): Light? = Light.entries.find { it.toString() == text }

fun showParse(text: String) {
    println("parse \"$text\": ${parseLight(text) ?: "not a light"}")
}

fun main() {
    println("lights: ${Light.entries.joinToString(" ")}")
    println("yellow is number ${Light.YELLOW.ordinal}")

    var light = Light.RED
    repeat(Light.entries.size) {
        println("after $light: ${light.next}")
        light = light.next
    }
    println(
        "red means ${Light.RED.meaning}, green means ${Light.GREEN.meaning}, " +
            "yellow means ${Light.YELLOW.meaning}"
    )

    println("one cycle: ${Light.entries.sumOf { it.seconds }} seconds")

    showParse("green")
    showParse("blue")
}
