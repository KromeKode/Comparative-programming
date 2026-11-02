class CalcError(message: String) : Exception(message)

// What eval needs from a number type N. The bound N : Arithmetic<N> makes
// every operator take and return the same type. div throws CalcError when
// dividing by zero.
interface Arithmetic<N : Arithmetic<N>> {
    operator fun plus(other: N): N

    operator fun minus(other: N): N

    operator fun times(other: N): N

    operator fun div(other: N): N
}

// An interface can't require a static function, so parsing lives in a
// separate object. Each number type's companion object is its Parser.
fun interface Parser<N> {
    fun parse(text: String): N?
}

val whole = Regex("-?[0-9]+")

fun parseWhole(text: String): Long? = if (whole.matches(text)) text.toLongOrNull() else null

data class IntNumber(val value: Long) : Arithmetic<IntNumber> {
    override fun plus(other: IntNumber) = IntNumber(value + other.value)

    override fun minus(other: IntNumber) = IntNumber(value - other.value)

    override fun times(other: IntNumber) = IntNumber(value * other.value)

    // Kotlin's / rounds toward zero.
    override fun div(other: IntNumber): IntNumber {
        if (other.value == 0L) throw CalcError("division by zero")
        return IntNumber(value / other.value)
    }

    override fun toString() = value.toString()

    companion object : Parser<IntNumber> {
        override fun parse(text: String) = parseWhole(text)?.let(::IntNumber)
    }
}

// Always in lowest terms, with a positive bottom; the constructor is private
// so that every Frac goes through of.
class Frac private constructor(val top: Long, val bottom: Long) : Arithmetic<Frac> {
    override fun plus(other: Frac) =
        of(top * other.bottom + other.top * bottom, bottom * other.bottom)

    override fun minus(other: Frac) =
        of(top * other.bottom - other.top * bottom, bottom * other.bottom)

    override fun times(other: Frac) = of(top * other.top, bottom * other.bottom)

    override fun div(other: Frac): Frac {
        if (other.top == 0L) throw CalcError("division by zero")
        return of(top * other.bottom, bottom * other.top)
    }

    override fun toString() = if (bottom == 1L) "$top" else "$top/$bottom"

    companion object : Parser<Frac> {
        tailrec fun gcd(a: Long, b: Long): Long = if (b == 0L) Math.abs(a) else gcd(b, a % b)

        fun of(top: Long, bottom: Long): Frac {
            val sign = if (bottom < 0) -1 else 1
            val divisor = gcd(top, bottom)
            return Frac(sign * top / divisor, sign * bottom / divisor)
        }

        override fun parse(text: String): Frac? {
            val parts = text.split("/")
            val top = parseWhole(parts[0]) ?: return null
            val bottom = if (parts.size == 1) 1L else parseWhole(parts[1]) ?: return null
            // "1/-2" and "1/0" are not fractions: the bottom must be above zero.
            return if (parts.size <= 2 && bottom > 0) of(top, bottom) else null
        }
    }
}

// Always 0 to 6. mod keeps the sign of 7, so -1 mod 7 is 6, where -1 % 7 is -1.
class Mod7 private constructor(val value: Long) : Arithmetic<Mod7> {
    override fun plus(other: Mod7) = of(value + other.value)

    override fun minus(other: Mod7) = of(value - other.value)

    override fun times(other: Mod7) = of(value * other.value)

    // x to the power 5 is the inverse of x when 7 is prime: x * x^5 = x^6 = 1.
    override fun div(other: Mod7): Mod7 {
        if (other.value == 0L) throw CalcError("division by zero")
        return this * other * other * other * other * other
    }

    override fun toString() = value.toString()

    companion object : Parser<Mod7> {
        fun of(n: Long) = Mod7(n.mod(7L))

        override fun parse(text: String) = parseWhole(text)?.let(::of)
    }
}

// Run the tokens on a stack of N. The same code works for every number type.
fun <N : Arithmetic<N>> eval(tokens: List<String>, parser: Parser<N>): N {
    val stack = ArrayDeque<N>()
    for (token in tokens) {
        if (token in setOf("+", "-", "*", "/")) {
            if (stack.size < 2) throw CalcError("not enough numbers")
            val right = stack.removeLast()
            val left = stack.removeLast()
            stack.addLast(
                when (token) {
                    "+" -> left + right
                    "-" -> left - right
                    "*" -> left * right
                    else -> left / right
                }
            )
        } else {
            stack.addLast(parser.parse(token) ?: throw CalcError("bad token \"$token\""))
        }
    }
    if (stack.size != 1) throw CalcError("expected one result, found ${stack.size}")
    return stack.single()
}

fun main() {
    for (line in generateSequence(::readlnOrNull)) {
        val words = line.split(" ").filter { it.isNotEmpty() }
        if (words.isEmpty()) continue
        val tokens = words.drop(1)
        try {
            // A class name used as a value means its companion object: the Parser.
            val result =
                when (words[0]) {
                    "int" -> eval(tokens, IntNumber)
                    "frac" -> eval(tokens, Frac)
                    "mod7" -> eval(tokens, Mod7)
                    else -> throw CalcError("unknown kind \"${words[0]}\"")
                }
            println(result)
        } catch (error: CalcError) {
            println("error: ${error.message}")
        }
    }
}
