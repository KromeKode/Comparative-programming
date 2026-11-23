// Evaluates arithmetic with +, -, *, and parentheses: each argument, or each line of
// input if there are none. Section 20.5 runs it in a debugger, to watch the parser
// call itself. It expects well-formed expressions, and doesn't check.

/** A recursive descent parser, with one method for each rule of the grammar. */
class Parser(private val text: String) {
    private var pos = 0

    /** Skips spaces, and gives the next character, or '\u0000' at the end. */
    private fun peek(): Char {
        while (pos < text.length && text[pos] == ' ') pos++
        return if (pos < text.length) text[pos] else '\u0000'
    }

    // expr = term { ("+" | "-") term }
    fun expr(): Long {
        var value = term()
        while (peek() == '+' || peek() == '-') {
            val op = text[pos++]
            val right = term()
            value = if (op == '+') value + right else value - right
        }
        return value
    }

    // term = factor { "*" factor }
    private fun term(): Long {
        var value = factor()
        while (peek() == '*') {
            pos++
            value *= factor()
        }
        return value
    }

    // factor = number | "(" expr ")"
    private fun factor(): Long {
        if (peek() == '(') {
            pos++
            val value = expr()
            peek()
            pos++ // the ")"
            return value
        }
        val start = pos
        while (pos < text.length && text[pos].isDigit()) pos++
        return text.substring(start, pos).toLong()
    }
}

fun evaluate(text: String): Long = Parser(text).expr()

fun main(args: Array<String>) {
    for (text in args) println(evaluate(text))
    if (args.isNotEmpty()) return
    generateSequence(::readlnOrNull).forEach { println(evaluate(it)) }
}
