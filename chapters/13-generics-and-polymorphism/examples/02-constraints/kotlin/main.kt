// Money keeps whole cents, so adding never rounds. Comparable<Money> gives it
// an order, and with it <, >, and the bound on describeRange below.
data class Money(val cents: Long) : Comparable<Money> {
    operator fun plus(other: Money) = Money(cents + other.cents)

    override fun compareTo(other: Money) = cents.compareTo(other.cents)

    override fun toString() = "$" + cents / 100 + "." + (cents % 100).toString().padStart(2, '0')
}

// No interface says "has +": Int's plus and Money's plus are unrelated
// functions. So total takes an Adder, an object that knows how to add Ts.
interface Adder<T> {
    val zero: T

    fun add(a: T, b: T): T
}

object IntAdder : Adder<Int> {
    override val zero = 0

    override fun add(a: Int, b: Int) = a + b
}

object DoubleAdder : Adder<Double> {
    override val zero = 0.0

    override fun add(a: Double, b: Double) = a + b
}

object MoneyAdder : Adder<Money> {
    override val zero = Money(0)

    override fun add(a: Money, b: Money) = a + b
}

fun <T> total(items: List<T>, adder: Adder<T>): T = items.fold(adder.zero, adder::add)

// Int, Double, and String are already Comparable to themselves.
fun <T : Comparable<T>> describeRange(items: List<T>): String {
    var (low, high) = items[0] to items[0]
    for (item in items) {
        if (item < low) low = item
        if (item > high) high = item
    }
    return "$low to $high"
}

fun <T> list(items: List<T>) = items.joinToString(", ")

fun main() {
    val numbers = listOf(1, 2, 3)
    val halves = listOf(0.5, 0.25)
    val prices = listOf(Money(150), Money(225))
    println("total of ${list(numbers)}: ${total(numbers, IntAdder)}")
    println("total of ${list(halves)}: ${total(halves, DoubleAdder)}")
    println("total of ${list(prices)}: ${total(prices, MoneyAdder)}")

    val digits = listOf(3, 9, 1)
    val fruit = listOf("pear", "fig", "apple")
    val costs = listOf(Money(200), Money(99))
    println("range of ${list(digits)}: ${describeRange(digits)}")
    println("range of ${list(fruit)}: ${describeRange(fruit)}")
    println("range of ${list(costs)}: ${describeRange(costs)}")
}
