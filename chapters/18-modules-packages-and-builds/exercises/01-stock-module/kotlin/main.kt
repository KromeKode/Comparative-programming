// Exercise 18.1: a stock module. This file is given: write stock.kt so that it
// builds. It reads the commands, and leaves keeping count to the stock package.
import stock.Stock

// An amount is a whole number from 1 to 999,999,999,999,999.
private val amountPattern = Regex("0*[1-9][0-9]{0,14}")

fun main() {
    val stock = Stock()
    generateSequence(::readlnOrNull).forEachIndexed { index, raw ->
        val number = index + 1
        val words = raw.split(Regex("\\s+")).filter { it.isNotEmpty() }
        if (words.isEmpty()) return@forEachIndexed
        val amount = words.getOrNull(2)?.takeIf { words.size == 3 && amountPattern.matches(it) }
            ?.toLong()
        val item = words.getOrNull(1) ?: ""
        when {
            amount != null && words[0] == "add" -> {
                stock.add(item, amount)
                println("added $amount $item, now ${stock.count(item)}")
            }
            amount != null && words[0] == "take" ->
                if (stock.take(item, amount)) {
                    println("took $amount $item, now ${stock.count(item)}")
                } else {
                    println("error: line $number: only ${stock.count(item)} $item in stock")
                }
            words.size == 2 && words[0] == "count" -> println("$item: ${stock.count(item)}")
            words.size == 1 && words[0] == "list" -> {
                val items = stock.items().map { (name, count) -> "$name $count" }
                println("stock: ${if (items.isEmpty()) "empty" else items.joinToString(", ")}")
            }
            else -> println("error: line $number: expected add, take, count, or list")
        }
    }
}
