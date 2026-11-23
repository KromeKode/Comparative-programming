// Reads one JSON text on stdin, strictly as RFC 8259 has it, and prints it compactly:
// no spaces, keys in the order they came, numbers as they were written, and strings
// written again with only the escapes they need. Or prints the first error's line and
// column, and exits with 1.
import kotlin.system.exitProcess

const val MAX_DEPTH = 1000

/** s as a JSON string, with only the escapes it needs. */
fun quote(s: String): String = buildString {
    append('"')
    for (c in s) {
        when {
            c == '"' -> append("\\\"")
            c == '\\' -> append("\\\\")
            c == '\b' -> append("\\b")
            c == '\u000c' -> append("\\f")
            c == '\n' -> append("\\n")
            c == '\r' -> append("\\r")
            c == '\t' -> append("\\t")
            c < ' ' -> append("\\u" + c.code.toString(16).padStart(4, '0'))
            else -> append(c)
        }
    }
    append('"')
}

class JsonError(val at: Int, message: String) : Exception(message) // at indexes code points

const val END = -1 // stands for the end of the text

class Parser(text: String) {
    // A String is UTF-16, so these are its code points, and an index counts them, as
    // columns do.
    val text: IntArray = text.codePoints().toArray()
    private var i = 0
    private var depth = 0

    private fun peek() = if (i < text.size) text[i] else END

    private fun at(c: Char) = peek() == c.code

    private fun fail(expected: String): JsonError {
        val found = if (peek() == END) "the end of the input" else quote(Character.toString(peek()))
        return JsonError(i, "expected $expected, found $found")
    }

    private fun space() {
        while (at(' ') || at('\t') || at('\n') || at('\r')) i++
    }

    private fun startsWith(at: Int, word: String) =
        at + word.length <= text.size && String(text, at, word.length) == word

    private fun value(): String {
        if (at('{') || at('[')) {
            depth++
            if (depth > MAX_DEPTH) throw JsonError(i, "nesting deeper than $MAX_DEPTH")
            val out = if (at('{')) obj() else array()
            depth--
            return out
        }
        if (at('"')) return quote(string())
        if (at('-') || isDigit()) return number()
        for (word in listOf("true", "false", "null")) {
            if (startsWith(i, word)) {
                i += word.length
                return word
            }
        }
        throw fail("a value")
    }

    private fun obj(): String {
        i++ // the {
        space()
        if (at('}')) {
            i++
            return "{}"
        }
        val members = mutableListOf<String>()
        val keys = mutableSetOf<String>()
        while (true) {
            if (!at('"')) {
                throw fail(if (members.isEmpty()) "a string key or \"}\"" else "a string key")
            }
            val start = i
            val key = string()
            // add returns false if the set had it already.
            if (!keys.add(key)) throw JsonError(start, "duplicate key ${quote(key)}")
            space()
            if (!at(':')) throw fail("\":\"")
            i++
            space()
            members.add("${quote(key)}:${value()}")
            space()
            if (at('}')) {
                i++
                return members.joinToString(",", "{", "}")
            }
            if (!at(',')) throw fail("\",\" or \"}\"")
            i++
            space()
        }
    }

    private fun array(): String {
        i++ // the [
        space()
        if (at(']')) {
            i++
            return "[]"
        }
        val items = mutableListOf<String>()
        while (true) {
            items.add(value())
            space()
            if (at(']')) {
                i++
                return items.joinToString(",", "[", "]")
            }
            if (!at(',')) throw fail("\",\" or \"]\"")
            i++
            space()
        }
    }

    private fun isDigit() = peek() in '0'.code..'9'.code

    private fun digits() {
        if (!isDigit()) throw fail("a digit")
        while (isDigit()) i++
    }

    private fun number(): String {
        val start = i
        if (at('-')) i++
        if (at('0')) i++ // a 0 can't have digits after it
        else digits()
        if (at('.')) {
            i++
            digits()
        }
        if (at('e') || at('E')) {
            i++
            if (at('+') || at('-')) i++
            digits()
        }
        return String(text, start, i - start) // as it was written
    }

    /** The four hex digits at text[at], or null if they aren't. */
    private fun hex4(at: Int): Int? {
        if (at + 4 > text.size) return null
        val digits = String(text, at, 4)
        return if (digits.all { it in "0123456789abcdefABCDEF" }) digits.toInt(16) else null
    }

    private val simple =
        mapOf('"' to '"', '\\' to '\\', '/' to '/', 'b' to '\b', 'f' to '\u000c') +
            mapOf('n' to '\n', 'r' to '\r', 't' to '\t')

    /** The string at i, with its escapes undone. */
    private fun string(): String {
        val start = i
        i++ // the "
        val out = StringBuilder()
        while (true) {
            val c = peek()
            if (c == END) throw JsonError(start, "string is never closed")
            if (at('"')) {
                i++
                return out.toString()
            }
            if (c < 0x20) throw JsonError(i, "control character in a string")
            if (!at('\\')) {
                out.appendCodePoint(c)
                i++
                continue
            }
            if (i + 1 == text.size) throw JsonError(start, "string is never closed")
            val escape = text[i + 1]
            val short = if (escape < 0x80) simple[escape.toChar()] else null
            if (short != null) {
                out.append(short)
                i += 2
                continue
            }
            val hex = if (escape == 'u'.code) hex4(i + 2) else null
            var code = hex ?: throw JsonError(i, "invalid escape")
            val escapeAt = i
            i += 6
            if (code in 0xDC00..0xDFFF) throw JsonError(escapeAt, "unpaired surrogate")
            if (code in 0xD800..0xDBFF) {
                // A character above U+FFFF is two escapes, a high surrogate and a low one.
                val low = if (startsWith(i, "\\u")) hex4(i + 2) else null
                if (low == null || low !in 0xDC00..0xDFFF) {
                    throw JsonError(escapeAt, "unpaired surrogate")
                }
                code = 0x10000 + ((code - 0xD800) shl 10) + (low - 0xDC00)
                i += 6
            }
            out.appendCodePoint(code)
        }
    }

    fun parse(): String {
        space()
        val out = value()
        space()
        if (i < text.size) throw fail("the end of the input")
        return out
    }
}

fun main() {
    val parser = Parser(System.`in`.readBytes().decodeToString())
    try {
        println(parser.parse())
    } catch (err: JsonError) {
        // Lines end at "\n", and columns count characters from 1.
        val before = parser.text.take(err.at)
        val line = before.count { it == '\n'.code } + 1
        val column = err.at - before.lastIndexOf('\n'.code)
        System.err.println("jsoncheck: line $line, column $column: ${err.message}")
        exitProcess(1)
    }
}
