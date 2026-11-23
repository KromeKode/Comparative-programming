// Reads CSV, as RFC 4180 describes it, and prints each record as a JSON list of strings,
// one record on each line. A field in quotes can hold commas, line breaks, and "" for a
// quote. Lines can end with "\r\n" or "\n". Every record must have as many fields as the
// first. At the first error, it prints the line and column, and the exit status is 1.
import kotlin.system.exitProcess

enum class State {
    RECORD_START, // nothing of this record read yet
    FIELD_START, // just after a comma
    UNQUOTED, // in a field that doesn't start with a quote
    QUOTED, // in a field that does
    QUOTE, // just after a quote in a quoted field: its end, or the first of ""
    CR, // just after a "\r" outside quotes, which needs a "\n" next
}

class CsvError(line: Int, column: Int, message: String) :
    Exception("line $line, column $column: $message")

fun fields(n: Int) = if (n == 1) "1 field" else "$n fields"

/** The records in text, one at a time. Throws a CsvError at the first error. */
fun records(text: String): Sequence<List<String>> = sequence {
    var state = State.RECORD_START
    var record = mutableListOf<String>()
    val field = StringBuilder()
    var width = 0 // how many fields the first record has, once it has ended
    var line = 1
    var column = 0 // where char is: columns count characters, from 1
    var start = Pair(0, 0) // where the quote that began a quoted field is, or the "\r"
    var recordLine = 1
    // A String is UTF-16, and codePoints() goes through its code points, so "🦊" is one
    // character. The null after them stands for the end, so the end goes through too.
    val chars = text.codePoints().toArray().map { Character.toString(it) } + null
    for (char in chars) {
        column++
        if (state == State.RECORD_START) recordLine = line
        var ended = false // whether char ends a record
        // The first branch whose condition holds is taken.
        when {
            state == State.QUOTED && char == null ->
                throw CsvError(start.first, start.second, "quoted field is never closed")
            state == State.QUOTED && char == "\"" -> state = State.QUOTE
            state == State.QUOTED -> field.append(char)
            state == State.QUOTE && char == "\"" -> {
                field.append('"')
                state = State.QUOTED
            }
            state == State.CR && char == "\n" -> ended = true
            state == State.CR ->
                throw CsvError(
                    start.first,
                    start.second,
                    "a carriage return without a line feed after it",
                )
            state == State.RECORD_START && char == null -> {}
            (state == State.RECORD_START || state == State.FIELD_START) && char == "\"" -> {
                start = Pair(line, column)
                state = State.QUOTED
            }
            state == State.UNQUOTED && char == "\"" ->
                throw CsvError(line, column, "a quote in an unquoted field")
            // From here on, the state is RECORD_START, FIELD_START, UNQUOTED, or QUOTE.
            char == "," -> {
                record.add(field.toString())
                field.clear()
                state = State.FIELD_START
            }
            char == "\n" || char == null -> ended = true
            char == "\r" -> {
                start = Pair(line, column)
                state = State.CR
            }
            state == State.QUOTE ->
                throw CsvError(line, column, "expected a comma or a line end after a closing quote")
            else -> {
                field.append(char)
                state = State.UNQUOTED
            }
        }
        if (ended) {
            record.add(field.toString())
            field.clear()
            if (width == 0) {
                width = record.size
            } else if (record.size != width) {
                val message = "record has ${fields(record.size)}, but the first has $width"
                throw CsvError(recordLine, 1, message)
            }
            yield(record)
            record = mutableListOf()
            state = State.RECORD_START
        }
        if (char == "\n") {
            line++
            column = 0
        }
    }
}

/** s as a JSON string, as JavaScript's JSON.stringify writes it. */
fun json(s: String): String = buildString {
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
            c < ' ' -> append("\\u%04x".format(c.code))
            else -> append(c)
        }
    }
    append('"')
}

fun main() {
    val text = System.`in`.readBytes().decodeToString()
    // A BufferedWriter writes in big pieces, and in UTF-8.
    val out = System.out.bufferedWriter()
    try {
        for (record in records(text)) {
            out.write(record.joinToString(",", "[", "]\n") { json(it) })
        }
    } catch (err: CsvError) {
        out.flush()
        System.err.println("csv2json: ${err.message}")
        exitProcess(1)
    }
    out.flush()
}
