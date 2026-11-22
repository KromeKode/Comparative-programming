import kotlin.reflect.KParameter
import kotlin.reflect.full.findAnnotation
import kotlin.reflect.full.memberProperties
import kotlin.reflect.full.primaryConstructor

// An annotation is data attached to a declaration, which reflection can
// read. Column gives a field the name its column has in the table.
@Target(AnnotationTarget.VALUE_PARAMETER) annotation class Column(val name: String)

// The fields are written once, here, with their types and defaults.
data class Book(
    val title: String = "",
    val author: String = "",
    val year: Long = 0,
    val pages: Long = 0,
    @Column("in_print") val inPrint: Boolean = false,
)

class BadValue(message: String) : Exception(message)

val parameters = Book::class.primaryConstructor!!.parameters

fun columnName(parameter: KParameter) = parameter.findAnnotation<Column>()?.name ?: parameter.name!!

val byColumn = parameters.associateBy(::columnName)

val whole = Regex("-?[0-9]+")

// The parameter's type says which parser to use.
fun parse(parameter: KParameter, text: String): Any {
    val name = columnName(parameter)
    return when (parameter.type.classifier) {
        String::class -> text
        Long::class ->
            if (whole.matches(text)) text.toLong()
            else throw BadValue("$name: \"$text\" is not a whole number")
        Boolean::class ->
            text.toBooleanStrictOrNull() ?: throw BadValue("$name: \"$text\" is not true or false")
        else -> error("no parser for $name")
    }
}

fun describe(book: Book): String {
    val properties = Book::class.memberProperties.associateBy { it.name }
    val parts = parameters.map { parameter ->
        val value = properties.getValue(parameter.name!!).get(book)
        columnName(parameter) + "=" + if (value is String) "\"$value\"" else "$value"
    }
    return "Book(${parts.joinToString(", ")})"
}

// callBy builds a Book from the parameters it is given, and fills in the
// defaults of the ones it isn't.
fun readBook(columns: List<KParameter>, values: List<String>): Book {
    if (values.size != columns.size) {
        throw BadValue("expected ${columns.size} values, got ${values.size}")
    }
    val arguments =
        columns.zip(values).associate { (parameter, text) -> parameter to parse(parameter, text) }
    return Book::class.primaryConstructor!!.callBy(arguments)
}

fun split(line: String) = line.split(",").map { it.trim() }

fun plural(count: Int, word: String) = "$count $word" + if (count == 1) "" else "s"

fun main() {
    val lines = generateSequence(::readlnOrNull).filter { it.isNotBlank() }.toList()
    if (lines.isEmpty()) return
    val names = split(lines.first())
    for ((i, name) in names.withIndex()) {
        if (name !in byColumn) {
            println("unknown column: $name")
            return
        }
        if (name in names.take(i)) {
            println("repeated column: $name")
            return
        }
    }
    val columns = names.map { byColumn.getValue(it) }

    var books = 0
    var bad = 0
    for ((i, line) in lines.drop(1).withIndex()) {
        try {
            println(describe(readBook(columns, split(line))))
            books++
        } catch (error: BadValue) {
            println("row ${i + 1}: ${error.message}")
            bad++
        }
    }
    println("${plural(books, "book")}, ${plural(bad, "bad row")}")
}
