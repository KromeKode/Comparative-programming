// Any parameter can have a default, and any argument can be passed by name.
fun greet(name: String, greeting: String = "Hello", punctuation: String = "!") =
    "$greeting, $name$punctuation"

// vararg collects any number of arguments into an array (an IntArray here).
fun total(vararg values: Int): Int {
    var sum = 0
    for (value in values) {
        sum += value
    }
    return sum
}

fun main() {
    println(greet("Ada"))
    println(greet("Grace", "Hi"))
    println(greet("Linus", punctuation = "?")) // a named argument skips greeting

    println("total() = ${total()}")
    println("total(1, 2, 3) = ${total(1, 2, 3)}")
    val numbers = intArrayOf(4, 3, 2, 1)
    println("total of [4, 3, 2, 1] = ${total(*numbers)}") // * spreads an array
}
