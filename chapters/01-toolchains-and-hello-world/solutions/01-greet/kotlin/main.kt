fun main() {
    val name = (readlnOrNull() ?: "").trim()
    if (name.isEmpty()) {
        println("Hello, stranger!")
    } else {
        println("Hello, $name!")
    }
}
