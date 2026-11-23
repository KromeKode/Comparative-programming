// Prints how one JSON file differs from another: what was removed, what was added, and
// what changed. ../README.md says what it should print.
import java.io.IOException
import java.nio.file.Files
import java.nio.file.NoSuchFileException
import java.nio.file.Path
import kotlin.system.exitProcess
import kotlinx.serialization.SerializationException
import kotlinx.serialization.json.Json
import kotlinx.serialization.json.JsonArray
import kotlinx.serialization.json.JsonElement
import kotlinx.serialization.json.JsonNull
import kotlinx.serialization.json.JsonObject
import kotlinx.serialization.json.JsonPrimitive

const val USAGE = "usage: jsondiff OLD NEW"
val IDENTIFIER = Regex("[A-Za-z_][A-Za-z0-9_]*")
val NUMBER = Regex("-?(0|[1-9][0-9]*)(\\.[0-9]+)?([eE][+-]?[0-9]+)?") // as RFC 8259 has it

// What strerror says for the errors the README names. Java's NoSuchFileException has
// only the path as its message.
fun message(error: IOException): String =
    when (error) {
        is NoSuchFileException -> "No such file or directory"
        else -> error.message ?: error.toString() // "Is a directory", for a folder
    }

// parseToJsonElement takes any word outside quotes as a value, such as NaN or yes, and
// makes it a JsonPrimitive whose isString is false. JSON's only words are true, false,
// null, and numbers.
fun isJson(element: JsonElement): Boolean =
    when (element) {
        is JsonObject -> element.values.all(::isJson)
        is JsonArray -> element.all(::isJson)
        is JsonNull -> true
        is JsonPrimitive ->
            element.isString || element.content in setOf("true", "false") ||
                NUMBER.matches(element.content)
    }

// A JsonPrimitive's toString writes it as JSON, a string in quotes and with escapes.
fun compact(element: JsonElement): String =
    when (element) {
        is JsonObject ->
            element.keys.sorted().joinToString(",", "{", "}") { key ->
                "${JsonPrimitive(key)}:${compact(element.getValue(key))}"
            }
        is JsonArray -> element.joinToString(",", "[", "]", transform = ::compact)
        is JsonPrimitive -> element.toString()
    }

// The part of a path that leads to a key: .name, or ["first name"].
fun step(key: String): String = if (IDENTIFIER.matches(key)) ".$key" else "[${JsonPrimitive(key)}]"

// Adds a line to out for each difference between before and after.
fun diff(path: String, before: JsonElement, after: JsonElement, out: MutableList<String>) {
    when {
        before is JsonObject && after is JsonObject ->
            // sorted() compares Strings by their characters' codes.
            for (key in (before.keys + after.keys).sorted()) {
                val here = path + step(key)
                when (key) {
                    !in after -> out.add("- $here: ${compact(before.getValue(key))}")
                    !in before -> out.add("+ $here: ${compact(after.getValue(key))}")
                    else -> diff(here, before.getValue(key), after.getValue(key), out)
                }
            }
        before is JsonArray && after is JsonArray ->
            for (i in 0 until maxOf(before.size, after.size)) {
                val here = "$path[$i]"
                when {
                    i >= after.size -> out.add("- $here: ${compact(before[i])}")
                    i >= before.size -> out.add("+ $here: ${compact(after[i])}")
                    else -> diff(here, before[i], after[i], out)
                }
            }
        // Two JsonPrimitives are equal if both or neither are strings, with the same text.
        before != after -> out.add("~ $path: ${compact(before)} -> ${compact(after)}")
    }
}

fun main(args: Array<String>) {
    if (args.size != 2) {
        System.err.println(USAGE)
        exitProcess(2)
    }
    val values =
        args.map { path ->
            val text =
                try {
                    Files.readAllBytes(Path.of(path)).decodeToString()
                } catch (error: IOException) {
                    System.err.println("jsondiff: $path: ${message(error)}")
                    exitProcess(2)
                }
            val value =
                try {
                    Json.parseToJsonElement(text)
                } catch (_: SerializationException) {
                    null
                }
            if (value == null || !isJson(value)) {
                System.err.println("jsondiff: $path: not valid JSON")
                exitProcess(2)
            }
            value
        }
    val lines = mutableListOf<String>()
    diff("$", values[0], values[1], lines)
    lines.forEach(::println)
    exitProcess(if (lines.isEmpty()) 0 else 1)
}
