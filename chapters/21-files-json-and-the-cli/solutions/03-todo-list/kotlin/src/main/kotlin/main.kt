// Keeps a to-do list in todo.json, in the current folder: adds tasks, lists them, marks
// them done, and removes them. ../README.md says what it should do.
import java.io.FileOutputStream
import java.io.IOException
import java.nio.file.Files
import java.nio.file.NoSuchFileException
import java.nio.file.Path
import java.nio.file.StandardCopyOption
import kotlin.system.exitProcess
import kotlinx.serialization.ExperimentalSerializationApi
import kotlinx.serialization.KSerializer
import kotlinx.serialization.Serializable
import kotlinx.serialization.SerializationException
import kotlinx.serialization.descriptors.PrimitiveKind
import kotlinx.serialization.descriptors.PrimitiveSerialDescriptor
import kotlinx.serialization.encoding.Decoder
import kotlinx.serialization.encoding.Encoder
import kotlinx.serialization.json.Json
import kotlinx.serialization.json.JsonArray
import kotlinx.serialization.json.JsonDecoder
import kotlinx.serialization.json.JsonElement
import kotlinx.serialization.json.JsonNull
import kotlinx.serialization.json.JsonObject
import kotlinx.serialization.json.JsonPrimitive
import kotlinx.serialization.json.booleanOrNull
import kotlinx.serialization.json.decodeFromJsonElement
import kotlinx.serialization.json.doubleOrNull
import kotlinx.serialization.json.intOrNull

const val FILE = "todo.json"
const val VERSION = 1
const val USAGE =
    """usage: todo add TEXT...
       todo list
       todo done ID
       todo remove ID"""
val DIGITS = Regex("[0-9]+")
val NUMBER = Regex("-?(0|[1-9][0-9]*)(\\.[0-9]+)?([eE][+-]?[0-9]+)?") // as RFC 8259 has it

// A problem with todo.json, which stops the program with status 2.
class Problem(message: String) : Exception(message)

// kotlinx.serialization reads a string such as "1" or "true" as a number or a boolean,
// and has no setting to stop it. These read the JSON value itself, and refuse a string.
fun unquoted(decoder: Decoder): JsonPrimitive? =
    ((decoder as JsonDecoder).decodeJsonElement() as? JsonPrimitive)?.takeIf { !it.isString }

object StrictIntSerializer : KSerializer<Int> {
    override val descriptor = PrimitiveSerialDescriptor("StrictInt", PrimitiveKind.INT)

    override fun serialize(encoder: Encoder, value: Int) = encoder.encodeInt(value)

    override fun deserialize(decoder: Decoder): Int =
        unquoted(decoder)?.intOrNull ?: throw SerializationException("expected a whole number")
}

object StrictBooleanSerializer : KSerializer<Boolean> {
    override val descriptor = PrimitiveSerialDescriptor("StrictBoolean", PrimitiveKind.BOOLEAN)

    override fun serialize(encoder: Encoder, value: Boolean) = encoder.encodeBoolean(value)

    override fun deserialize(decoder: Decoder): Boolean =
        unquoted(decoder)?.booleanOrNull ?: throw SerializationException("expected true or false")
}

// The fields are read and written in this order, under these names. A missing field is
// an error, as none of them has a default, and so is a key that no field has.
@Serializable
data class Task(
    @Serializable(with = StrictIntSerializer::class) val id: Int,
    val text: String,
    @Serializable(with = StrictBooleanSerializer::class) val done: Boolean,
)

@Serializable
data class TodoList(
    @Serializable(with = StrictIntSerializer::class) val version: Int,
    val tasks: List<Task>,
)

@OptIn(ExperimentalSerializationApi::class) // for prettyPrintIndent
val json = Json {
    prettyPrint = true
    prettyPrintIndent = "  " // four spaces, by default
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
            element.isString ||
                element.content in setOf("true", "false") ||
                NUMBER.matches(element.content)
    }

// What strerror says for the errors the README names. Java's NoSuchFileException has
// only the path as its message.
fun message(error: IOException): String =
    when (error) {
        is NoSuchFileException -> "No such file or directory"
        else -> error.message ?: error.toString() // "Is a directory", for a folder
    }

// The list in todo.json, or an empty one if there's no todo.json.
fun readList(): TodoList {
    val text =
        try {
            Files.readAllBytes(Path.of(FILE)).decodeToString()
        } catch (_: NoSuchFileException) {
            return TodoList(VERSION, emptyList())
        } catch (error: IOException) {
            throw Problem(message(error))
        }
    val element =
        try {
            Json.parseToJsonElement(text)
        } catch (_: SerializationException) {
            null
        }
    if (element == null || !isJson(element)) throw Problem("not valid JSON")
    // A later version of todo may keep anything else in the file, so the version comes
    // first. doubleOrNull is null for a string, true, false, or null.
    val version = (element as? JsonObject)?.get("version") as? JsonPrimitive
    val number = version?.takeIf { !it.isString }?.doubleOrNull
    if (number != null && number != VERSION.toDouble()) {
        throw Problem("unknown version ${version.content}")
    }
    val list =
        try {
            json.decodeFromJsonElement<TodoList>(element)
        } catch (_: SerializationException) {
            throw Problem("not a to-do list")
        }
    val ids = list.tasks.map { it.id }
    if (ids.any { it < 1 } || ids.toSet().size != ids.size) throw Problem("not a to-do list")
    return list
}

// Writes the list to todo.json. It writes a new file and renames it over the old one,
// which replaces it in one step, so that a crash leaves the old list or the new one, and
// never half of one.
fun writeList(list: TodoList) {
    val text = json.encodeToString(list) + "\n"
    val temporary = Path.of("$FILE.tmp")
    try {
        FileOutputStream(temporary.toFile()).use { out ->
            out.write(text.toByteArray()) // UTF-8
            out.fd.sync() // onto the disk before the rename
        }
        Files.move(temporary, Path.of(FILE), StandardCopyOption.REPLACE_EXISTING)
    } catch (error: IOException) {
        Files.deleteIfExists(temporary)
        throw Problem(message(error))
    }
}

// Carries out a command whose arguments have been checked, and returns the status.
fun run(command: String, args: List<String>): Int {
    val list = readList()
    when (command) {
        "add" -> {
            val added =
                Task((list.tasks.maxOfOrNull { it.id } ?: 0) + 1, args.joinToString(" "), false)
            writeList(list.copy(tasks = list.tasks + added))
            println("added ${added.id}: ${added.text}")
        }
        "list" ->
            for (task in list.tasks) {
                println("${task.id} [${if (task.done) "x" else " "}] ${task.text}")
            }
        else -> {
            val id = args[0].toIntOrNull() // null if it's too big for an Int
            val found = list.tasks.find { it.id == id }
            if (found == null) {
                System.err.println("todo: no task ${args[0]}")
                return 1
            }
            if (command == "done") {
                writeList(
                    list.copy(
                        tasks = list.tasks.map { if (it == found) it.copy(done = true) else it }
                    )
                )
                println("done ${found.id}: ${found.text}")
            } else {
                writeList(list.copy(tasks = list.tasks - found))
                println("removed ${found.id}: ${found.text}")
            }
        }
    }
    return 0
}

fun main(arguments: Array<String>) {
    val command = arguments.firstOrNull() ?: ""
    val args = arguments.drop(1)
    // The command line is checked before todo.json is read.
    val ok =
        when (command) {
            "add" -> args.isNotEmpty()
            "list" -> args.isEmpty()
            "done",
            "remove" -> args.size == 1 && DIGITS.matches(args[0])
            else -> false
        }
    if (!ok) {
        System.err.println(USAGE)
        exitProcess(2)
    }
    val status =
        try {
            run(command, args)
        } catch (problem: Problem) {
            System.err.println("todo: $FILE: ${problem.message}")
            2
        }
    exitProcess(status)
}
