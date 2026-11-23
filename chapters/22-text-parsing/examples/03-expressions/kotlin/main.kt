// Reads arithmetic, one expression on each line, such as 1 + 2 * 3, and prints each with
// its parentheses written out, and its value: (1 + (2 * 3)) = 7. The numbers are whole
// and fit in 64 bits; the operators are + - * / % ^ and unary -, with ( and ). / and %
// round toward zero, as in C. An error is printed with its line and a caret under its
// column, the next line is read, and at the end, the exit status is 1.
import kotlin.system.exitProcess

class CalcError(val column: Int, message: String) : Exception(message) // column from 1

// The lexer turns the text into tokens: numbers, and operators and parentheses, each
// with its column.
enum class Kind {
    NUMBER,
    OPERATOR, // or a parenthesis
    END, // after the last token
}

data class Token(val kind: Kind, val text: String, val column: Int, val value: Long = 0) {
    fun describe() = if (kind == Kind.END) "the end of the line" else "\"$text\""
}

fun tokens(line: String): List<Token> {
    // A String is UTF-16, so this splits it into code points, and i + 1 is the column.
    val chars = line.codePoints().toArray().map { Character.toString(it) }
    val found = mutableListOf<Token>()
    // Not Char.isDigit(), which is true for "٣", and toLong() takes "٣" as 3.
    fun isDigit(c: String) = c.length == 1 && c[0] in '0'..'9'
    var i = 0
    while (i < chars.size) {
        val c = chars[i]
        when {
            c == " " || c == "\t" -> i++
            isDigit(c) -> {
                val start = i
                while (i < chars.size && isDigit(chars[i])) i++
                val text = chars.subList(start, i).joinToString("")
                // toLongOrNull gives null if the number doesn't fit, where toLong throws.
                val value =
                    text.toLongOrNull()
                        ?: throw CalcError(start + 1, "number too large for 64 bits")
                found.add(Token(Kind.NUMBER, text, start + 1, value))
            }
            c in listOf("+", "-", "*", "/", "%", "^", "(", ")") -> {
                found.add(Token(Kind.OPERATOR, c, i + 1))
                i++
            }
            else -> throw CalcError(i + 1, "unexpected \"$c\"")
        }
    }
    found.add(Token(Kind.END, "", chars.size + 1))
    return found
}

// The parser turns the tokens into a tree, whose nodes are of three kinds. A sealed
// interface can have subtypes only in this module, so a when on an Expr can know that
// it has a branch for each. column is where the operator is, for an error such as
// overflow.
sealed interface Expr

data class Number(val value: Long) : Expr

data class Negate(val operand: Expr, val column: Int) : Expr

data class Binary(val op: String, val left: Expr, val right: Expr, val column: Int) : Expr

// How tightly each operator holds the expressions on its left and right: the higher,
// the tighter. Left is less than right for left-associative operators, so 1 - 2 - 3 is
// (1 - 2) - 3, and more for ^, so 2 ^ 3 ^ 2 is 2 ^ (3 ^ 2).
val INFIX =
    mapOf(
        "+" to Pair(1, 2),
        "-" to Pair(1, 2),
        "*" to Pair(3, 4),
        "/" to Pair(3, 4),
        "%" to Pair(3, 4),
        "^" to Pair(8, 7),
    )
// Unary minus holds less tightly than ^, so -2 ^ 2 is -(2 ^ 2), and more than *.
const val PREFIX = 5

class Parser(line: String) {
    private val tokens = tokens(line)
    private var at = 0

    private fun next(): Token {
        val token = tokens[at]
        if (token.kind != Kind.END) at++
        return token
    }

    /** Parses the longest expression whose operators hold at least minPower. */
    private fun expression(minPower: Int): Expr {
        val token = next()
        var left =
            when {
                token.kind == Kind.NUMBER -> Number(token.value)
                token.text == "-" -> Negate(expression(PREFIX), token.column)
                token.text == "(" -> {
                    val inside = expression(0)
                    val close = next()
                    if (close.text != ")") {
                        throw CalcError(close.column, "expected \")\", found ${close.describe()}")
                    }
                    inside
                }
                else -> {
                    val found = token.describe()
                    throw CalcError(
                        token.column,
                        "expected a number, \"(\", or \"-\", found $found",
                    )
                }
            }
        while (true) {
            val operator = tokens[at]
            val powers = if (operator.kind == Kind.OPERATOR) INFIX[operator.text] else null
            if (powers == null || powers.first < minPower) return left
            next()
            left = Binary(operator.text, left, expression(powers.second), operator.column)
        }
    }

    fun parse(): Expr {
        val expr = expression(0)
        val token = next()
        if (token.kind != Kind.END) {
            throw CalcError(token.column, "expected an operator, found ${token.describe()}")
        }
        return expr
    }
}

fun show(expr: Expr): String =
    when (expr) {
        is Number -> expr.value.toString()
        is Negate -> "(-${show(expr.operand)})"
        is Binary -> "(${show(expr.left)} ${expr.op} ${show(expr.right)})"
    }

// A Long wraps around when it overflows, with no error. Math's ...Exact functions throw
// an ArithmeticException instead, which this turns into an error at the column.
fun <T> fit(column: Int, work: () -> T): T =
    try {
        work()
    } catch (_: ArithmeticException) {
        throw CalcError(column, "overflow")
    }

// Squares the base for each bit of the exponent, and stops at the first overflow.
fun power(base: Long, exponent: Long, column: Int): Long {
    var (result, b, e) = Triple(1L, base, exponent)
    while (e > 0) {
        if (e % 2 == 1L) result = fit(column) { Math.multiplyExact(result, b) }
        e /= 2
        if (e > 0) b = fit(column) { Math.multiplyExact(b, b) }
    }
    return result
}

fun evaluate(expr: Expr): Long =
    when (expr) {
        is Number -> expr.value
        is Negate -> fit(expr.column) { Math.negateExact(evaluate(expr.operand)) }
        is Binary -> {
            val (a, b, column) = Triple(evaluate(expr.left), evaluate(expr.right), expr.column)
            if ((expr.op == "/" || expr.op == "%") && b == 0L) {
                throw CalcError(column, "division by zero")
            }
            when (expr.op) {
                "+" -> fit(column) { Math.addExact(a, b) }
                "-" -> fit(column) { Math.subtractExact(a, b) }
                "*" -> fit(column) { Math.multiplyExact(a, b) }
                // / and % round toward zero. Long.MIN_VALUE / -1 overflows, with no error,
                // to Long.MIN_VALUE; divideExact throws instead.
                "/" -> fit(column) { Math.divideExact(a, b) }
                "%" -> a % b
                else ->
                    if (b < 0) throw CalcError(column, "negative exponent") else power(a, b, column)
            }
        }
    }

fun main() {
    var status = 0
    val lines = System.`in`.readBytes().decodeToString().split("\n").toMutableList()
    // The last is empty when the input ended with a newline, or was empty.
    if (lines.last() == "") lines.removeLast()
    for ((index, text) in lines.withIndex()) {
        val line = text.removeSuffix("\r")
        if (line.trim(' ', '\t').isEmpty()) continue
        try {
            val expr = Parser(line).parse()
            println("${show(expr)} = ${evaluate(expr)}")
        } catch (err: CalcError) {
            // The caret goes under the column. A tab before it stays a tab, so that the
            // caret lines up with the text whatever a tab's width is.
            val before = line.codePoints().toArray().take(err.column - 1)
            val pad = before.joinToString("") { if (it == '\t'.code) "\t" else " " }
            System.err.println("calc: line ${index + 1}, column ${err.column}: ${err.message}")
            System.err.println("  $line\n  $pad^")
            status = 1
        }
    }
    exitProcess(status)
}
