data class Config(val name: String, val port: Int, val debug: Boolean, val timeout: Int)

// The value parsers throw ValueProblem; load wraps it in a BadValue.
class ValueProblem(message: String) : Exception(message)

// Every problem load can find. These are values in a list, not exceptions.
sealed class ConfigError {
    data class Syntax(val line: Int) : ConfigError()

    data class UnknownKey(val line: Int, val key: String) : ConfigError()

    data class DuplicateKey(val line: Int, val key: String, val first: Int) : ConfigError()

    data class BadValue(val line: Int, val key: String, val problem: ValueProblem) : ConfigError()

    data class MissingKey(val key: String) : ConfigError()

    val message: String
        get() =
            when (this) {
                is Syntax -> "line $line: expected \"key = value\""
                is UnknownKey -> "line $line: unknown key \"$key\""
                is DuplicateKey -> "line $line: \"$key\" is already set on line $first"
                is BadValue -> "line $line: $key: ${problem.message}"
                is MissingKey -> "missing key \"$key\""
            }

    val kind: String
        get() =
            when (this) {
                is Syntax -> "syntax"
                is UnknownKey -> "unknown key"
                is DuplicateKey -> "duplicate key"
                is BadValue -> "bad value"
                is MissingKey -> "missing key"
            }
}

val kinds = listOf("syntax", "unknown key", "duplicate key", "bad value", "missing key")

// What load returns: a config, or the list of everything wrong with the file.
sealed interface LoadResult {
    data class Loaded(val config: Config) : LoadResult

    data class Failed(val errors: List<ConfigError>) : LoadResult
}

fun parseText(text: String): String = text.ifEmpty { throw ValueProblem("must not be empty") }

fun parseWhole(text: String, low: Int, high: Int): Int {
    if (text.isEmpty() || !text.all { it in '0'..'9' }) {
        throw ValueProblem("\"$text\" is not a whole number")
    }
    // Stop growing past the limit, so a long number cannot overflow.
    val value = text.fold(0) { total, digit -> minOf(total * 10 + (digit - '0'), high + 1) }
    if (value !in low..high) throw ValueProblem("$text is not between $low and $high")
    return value
}

fun parseBool(text: String): Boolean =
    when (text) {
        "true" -> true
        "false" -> false
        else -> throw ValueProblem("\"$text\" is not true or false")
    }

fun load(lines: List<String>): LoadResult {
    var name: String? = null
    var port: Int? = null
    var debug = false
    var timeout = 30
    val firstLine = mutableMapOf<String, Int>()
    val errors = mutableListOf<ConfigError>()
    for ((index, raw) in lines.withIndex()) {
        val number = index + 1
        val line = raw.trim(' ')
        if (line.isEmpty() || line.startsWith("#")) continue
        val key = line.substringBefore('=', missingDelimiterValue = "").trim(' ')
        if (key.isEmpty()) {
            errors += ConfigError.Syntax(number)
            continue
        }
        val text = line.substringAfter('=').trim(' ')
        val first = firstLine[key]
        when {
            key !in listOf("name", "port", "debug", "timeout") ->
                errors += ConfigError.UnknownKey(number, key)
            first != null -> errors += ConfigError.DuplicateKey(number, key, first)
            else -> {
                firstLine[key] = number
                try {
                    when (key) {
                        "name" -> name = parseText(text)
                        "port" -> port = parseWhole(text, 1, 65535)
                        "debug" -> debug = parseBool(text)
                        else -> timeout = parseWhole(text, 1, 3600)
                    }
                } catch (problem: ValueProblem) {
                    errors += ConfigError.BadValue(number, key, problem)
                }
            }
        }
    }
    for (key in listOf("name", "port")) {
        if (key !in firstLine) errors += ConfigError.MissingKey(key)
    }
    val finalName = name
    val finalPort = port
    if (errors.isNotEmpty() || finalName == null || finalPort == null) {
        return LoadResult.Failed(errors)
    }
    return LoadResult.Loaded(Config(finalName, finalPort, debug, timeout))
}

fun main() {
    val files = mutableListOf<Pair<String, MutableList<String>>>()
    while (true) {
        val line = readlnOrNull() ?: break
        if (line.startsWith("== ")) {
            files += line.removePrefix("== ") to mutableListOf()
        } else {
            files.lastOrNull()?.second?.add(line)
        }
    }
    val counts = kinds.associateWith { 0 }.toMutableMap()
    var loaded = 0
    for ((name, lines) in files) {
        when (val result = load(lines)) {
            is LoadResult.Loaded -> {
                loaded++
                val config = result.config
                println("$name: ok")
                println("  name = ${config.name}")
                println("  port = ${config.port}")
                println("  debug = ${config.debug}")
                println("  timeout = ${config.timeout}")
            }
            is LoadResult.Failed -> {
                val count = result.errors.size
                println("$name: $count error${if (count == 1) "" else "s"}")
                for (error in result.errors) {
                    println("  ${error.message}")
                    counts[error.kind] = counts.getValue(error.kind) + 1
                }
            }
        }
    }
    println("files loaded: $loaded of ${files.size}")
    println(kinds.joinToString(", ") { "$it: ${counts[it]}" })
}
