// A fraction kept in lowest terms, with the sign on the numerator.
class Fraction(numerator: Long, denominator: Long) : Comparable<Fraction> {
    // private val: readable inside the class only, and never reassigned.
    private val numerator: Long
    private val denominator: Long // always positive

    init {
        require(denominator != 0L) { "denominator is zero" }
        val sign = if (denominator < 0) -1 else 1
        val divisor = gcd(numerator, denominator)
        this.numerator = sign * numerator / divisor
        this.denominator = sign * denominator / divisor
    }

    operator fun plus(other: Fraction) =
        Fraction(
            numerator * other.denominator + other.numerator * denominator,
            denominator * other.denominator,
        )

    // == calls equals, which compares identity unless a class overrides it.
    // A class that overrides equals must override hashCode to match.
    override fun equals(other: Any?) =
        other is Fraction && numerator == other.numerator && denominator == other.denominator

    override fun hashCode() = 31 * numerator.hashCode() + denominator.hashCode()

    // <, >, sorted, and maxOf all use compareTo.
    override fun compareTo(other: Fraction) =
        (numerator * other.denominator).compareTo(other.numerator * denominator)

    override fun toString() = if (denominator == 1L) "$numerator" else "$numerator/$denominator"

    private companion object {
        tailrec fun gcd(a: Long, b: Long): Long = if (b == 0L) Math.abs(a) else gcd(b, a % b)
    }
}

fun yesNo(value: Boolean) = if (value) "yes" else "no"

fun main() {
    val half = Fraction(1, 2)
    val third = Fraction(1, 3)
    println("1/2 + 1/3 = ${half + third}")
    println("2/-4 is stored as ${Fraction(2, -4)}")
    println("6/3 is stored as ${Fraction(6, 3)}")
    try {
        Fraction(1, 0)
    } catch (e: IllegalArgumentException) {
        println("1/0: error: ${e.message}")
    }
    println("1/2 == 2/4: ${yesNo(half == Fraction(2, 4))}")
    println("1/3 < 1/2: ${yesNo(third < half)}")
    val fractions = listOf(Fraction(3, 4), third, Fraction(-1, 2), Fraction(2, 4))
    println("sorted: ${fractions.sorted().joinToString(", ")}")
}
