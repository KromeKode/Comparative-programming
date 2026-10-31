import kotlin.math.PI

sealed interface Shape

data class Circle(val radius: Double) : Shape

data class Rectangle(val width: Double, val height: Double) : Shape

data class Triangle(val base: Double, val height: Double) : Shape

// The when must cover every implementation of the sealed Shape.
fun area(shape: Shape): Double =
    when (shape) {
        is Circle -> PI * shape.radius * shape.radius
        is Rectangle -> shape.width * shape.height
        is Triangle -> shape.base * shape.height / 2
    }

val sizesNeeded = mapOf("circle" to 1, "rectangle" to 2, "triangle" to 2)

fun parseSize(text: String): Double {
    val value = text.toDoubleOrNull() ?: throw IllegalArgumentException("\"$text\" is not a number")
    require(value >= 0) { "\"$text\" is negative" }
    return value
}

// Turns a shape name and its sizes into a Shape, or throws IllegalArgumentException.
fun parse(name: String, texts: List<String>): Shape {
    val needed = sizesNeeded[name] ?: throw IllegalArgumentException("unknown shape \"$name\"")
    require(texts.size == needed) {
        "$name needs $needed size${if (needed == 1) "" else "s"}, got ${texts.size}"
    }
    val sizes = texts.map(::parseSize)
    return when (name) {
        "circle" -> Circle(sizes[0])
        "rectangle" -> Rectangle(sizes[0], sizes[1])
        else -> Triangle(sizes[0], sizes[1])
    }
}

fun main() {
    var total = 0.0
    while (true) {
        val line = readlnOrNull() ?: break
        val words = line.split(Regex("\\s+")).filter { it.isNotEmpty() }
        if (words.isEmpty()) continue
        try {
            val shape = parse(words[0], words.drop(1))
            total += area(shape)
            println("${words[0]}: ${"%.2f".format(area(shape))}")
        } catch (error: IllegalArgumentException) {
            println("error: ${error.message}")
        }
    }
    println("total: ${"%.2f".format(total)}")
}
