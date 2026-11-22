// params is null for a macro that takes no parentheses.
data class Macro(val params: List<String>?, val body: String)

class ExpandError(message: String) : Exception(message)

val macros = mutableMapOf<String, Macro>()

val namePattern = Regex("[A-Za-z_][A-Za-z0-9_]*")

fun isWordChar(c: Char) = c in 'a'..'z' || c in 'A'..'Z' || c in '0'..'9' || c == '_'

fun isName(word: String) = namePattern.matches(word)

fun wordEnd(text: String, start: Int): Int {
    var end = start
    while (end < text.length && isWordChar(text[end])) end++
    return end
}

fun spacesEnd(text: String, start: Int): Int {
    var end = start
    while (end < text.length && text[end] == ' ') end++
    return end
}

// The index just past the string that starts at text[start], a quote.
fun stringEnd(text: String, start: Int): Int {
    val close = text.indexOf('"', start + 1)
    return if (close < 0) text.length else close + 1
}

// Splits the arguments of a call at its top-level commas, and trims them.
// Returns them with the index just past the ")".
fun splitArgs(text: String, open: Int, name: String): Pair<List<String>, Int> {
    val args = mutableListOf<String>()
    var depth = 0
    var start = open + 1
    var i = open + 1
    while (i < text.length) {
        val c = text[i]
        when {
            c == '"' -> {
                i = stringEnd(text, i)
                continue
            }
            c == '(' -> depth++
            c == ')' && depth > 0 -> depth--
            c == ')' -> {
                args.add(text.substring(start, i).trim())
                return args to i + 1
            }
            c == ',' && depth == 0 -> {
                args.add(text.substring(start, i).trim())
                start = i + 1
            }
        }
        i++
    }
    throw ExpandError("unterminated call to $name")
}

// The macro's body, with each parameter replaced by its argument.
fun substitute(macro: Macro, args: List<String>, disabled: Set<String>): String {
    val raw = macro.params!!.zip(args).toMap()
    val expanded = mutableMapOf<String, String>() // an argument is expanded only if it is used
    val body = macro.body
    val out = StringBuilder()
    var i = 0
    while (i < body.length) {
        var end = i + 1
        if (body[i] == '"') {
            end = stringEnd(body, i)
        } else if (body[i] == '#') {
            // # and a parameter make a string of the argument as written.
            val start = spacesEnd(body, i + 1)
            val stop = wordEnd(body, start)
            val arg = raw[body.substring(start, stop)]
            if (arg != null) {
                out.append('"').append(arg).append('"')
                i = stop
                continue
            }
        } else if (isWordChar(body[i])) {
            end = wordEnd(body, i)
            val word = body.substring(i, end)
            val arg = raw[word]
            if (arg != null) {
                out.append(expanded.getOrPut(word) { expand(arg, disabled) })
                i = end
                continue
            }
        }
        out.append(body, i, end)
        i = end
    }
    return out.toString()
}

// Expands every macro in text, except those in disabled.
fun expand(text: String, disabled: Set<String>): String {
    val out = StringBuilder()
    var i = 0
    while (i < text.length) {
        var end = i + 1
        if (text[i] == '"') {
            end = stringEnd(text, i)
        } else if (isWordChar(text[i])) {
            end = wordEnd(text, i)
            val word = text.substring(i, end)
            val macro = macros[word]
            if (macro != null && word !in disabled) {
                // Inside its own expansion, a macro's name is left alone.
                val inner = disabled + word
                val params = macro.params
                if (params == null) {
                    out.append(expand(macro.body, inner))
                    i = end
                    continue
                }
                val paren = spacesEnd(text, end)
                if (paren < text.length && text[paren] == '(') {
                    val (found, after) = splitArgs(text, paren, word)
                    val args = if (params.isEmpty() && found == listOf("")) emptyList() else found
                    val count = params.size
                    if (args.size != count) {
                        val noun = if (count == 1) "argument" else "arguments"
                        throw ExpandError("$word takes $count $noun, got ${args.size}")
                    }
                    out.append(expand(substitute(macro, args, disabled), inner))
                    i = after
                    continue
                }
            }
        }
        out.append(text, i, end)
        i = end
    }
    return out.toString()
}

fun define(rest: String) {
    val end = wordEnd(rest, 0)
    val name = rest.substring(0, end)
    if (!isName(name)) throw ExpandError("bad #define")
    if (rest.getOrNull(end) != '(') {
        macros[name] = Macro(null, rest.substring(end).trim())
        return
    }
    val close = rest.indexOf(')', end)
    if (close < 0) throw ExpandError("bad #define")
    val inside = rest.substring(end + 1, close)
    val params = if (inside.isBlank()) emptyList() else inside.split(",").map { it.trim() }
    if (!params.all(::isName) || params.toSet().size != params.size) {
        throw ExpandError("bad #define")
    }
    macros[name] = Macro(params, rest.substring(close + 1).trim())
}

// Returns the line to print, or null for a directive.
fun run(line: String): String? {
    val directive = line.trim().substringBefore(' ')
    val rest = line.trim().substringAfter(' ', "").trim()
    when (directive) {
        "#define" -> define(rest)
        "#undef" -> if (isName(rest)) macros.remove(rest) else throw ExpandError("bad #undef")
        else -> return expand(line, emptySet())
    }
    return null
}

fun main() {
    for (line in generateSequence(::readlnOrNull)) {
        try {
            run(line)?.let(::println)
        } catch (error: ExpandError) {
            println("error: ${error.message}")
        }
    }
}
