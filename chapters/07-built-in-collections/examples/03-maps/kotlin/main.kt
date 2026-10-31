// mutableMapOf makes a LinkedHashMap, which remembers the order keys were added in.
fun show(stock: Map<String, Int>) = stock.entries.joinToString(" ") { "${it.key}=${it.value}" }

fun main() {
    val stock = mutableMapOf("apple" to 3, "banana" to 5)
    println("stock: ${show(stock)}")

    stock["cherry"] = 7
    println("add cherry: ${show(stock)}")

    println("banana: ${stock["banana"]}")
    if ("durian" !in stock) {
        println("durian: ${stock["durian"] ?: 0} (missing)") // [] gives null if missing
    }

    stock["apple"] = stock.getValue("apple") + 2 // getValue throws if missing
    println("apple += 2: ${show(stock)}")
    stock.remove("banana")
    println("remove banana: ${show(stock)}")

    println("has cherry: ${if ("cherry" in stock) "yes" else "no"}")
    println("size: ${stock.size}")
}
