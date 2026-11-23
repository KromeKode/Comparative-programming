// Reads a JSON file of books and prints a report on them, as JSON: how many there are,
// the oldest, the books with each tag, and the books with none. The serialization
// plugin writes the code that turns JSON into a Library and a Report into JSON,
// checking every field's type on the way in.
import java.io.File
import java.io.IOException
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
import kotlinx.serialization.json.JsonDecoder
import kotlinx.serialization.json.JsonPrimitive
import kotlinx.serialization.json.intOrNull

// kotlinx.serialization reads a number in quotes, such as "1965", as a number, and has
// no setting to stop it. This reads the JSON value itself, and refuses a string.
object StrictIntSerializer : KSerializer<Int> {
    override val descriptor = PrimitiveSerialDescriptor("StrictInt", PrimitiveKind.INT)

    override fun serialize(encoder: Encoder, value: Int) = encoder.encodeInt(value)

    override fun deserialize(decoder: Decoder): Int {
        val element = (decoder as JsonDecoder).decodeJsonElement()
        val number = (element as? JsonPrimitive)?.takeIf { !it.isString }?.intOrNull
        return number ?: throw SerializationException("expected a whole number, not $element")
    }
}

@Serializable data class Library(val books: List<Book>)

@Serializable
data class Book(
    val title: String,
    val author: String,
    @Serializable(with = StrictIntSerializer::class) val year: Int,
    val tags: List<String> = emptyList(), // used when tags is missing
)

// The fields are written in this order, under these names.
@Serializable
data class Report(
    val count: Int,
    val oldest: String?, // null is written as null
    val tags: Map<String, List<String>>,
    val untagged: List<String>,
)

@OptIn(ExperimentalSerializationApi::class) // for prettyPrintIndent
val json = Json {
    ignoreUnknownKeys = true // by default, a field Book doesn't have is an error
    prettyPrint = true
    prettyPrintIndent = "  " // four spaces, by default
}

fun report(books: List<Book>): Report {
    val oldest = books.minByOrNull { it.year } // the first of the oldest
    val tags = sortedMapOf<String, MutableList<String>>() // keeps its keys in order
    for (book in books) {
        for (tag in book.tags) tags.getOrPut(tag) { mutableListOf() }.add(book.title)
    }
    return Report(
        count = books.size,
        oldest = oldest?.let { "${it.title}, by ${it.author}" },
        tags = tags,
        untagged = books.filter { it.tags.isEmpty() }.map { it.title },
    )
}

fun main(args: Array<String>) {
    if (args.size != 1) {
        System.err.println("usage: books FILE")
        exitProcess(2)
    }
    val path = args[0]
    val library =
        try {
            json.decodeFromString<Library>(File(path).readText())
        } catch (error: IOException) {
            System.err.println("books: ${error.message}") // the path and the reason
            exitProcess(1)
        } catch (error: SerializationException) {
            // The message is several lines, with the JSON around the problem.
            System.err.println("books: $path: ${error.message?.lines()?.first()}")
            exitProcess(1)
        }
    println(json.encodeToString(report(library.books)))
}
