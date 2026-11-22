import kotlin.reflect.full.primaryConstructor

// A data class. The compiler writes toString, equals, hashCode, and copy
// from the properties in its primary constructor.
data class Book(val title: String, val author: String, val year: Int, val inPrint: Boolean)

// Book::title is a property reference, which works as a function from a
// Book to its title. compareBy tries each one in turn.
val bookOrder = compareBy(Book::title, Book::author, Book::year, Book::inPrint)

fun main() {
    // Reflection, from the kotlin-reflect library, can list them as the
    // program runs.
    val fields = Book::class.primaryConstructor!!.parameters.map { it.name }
    println("fields: ${fields.joinToString(" ")}")

    val dune = Book("Dune", "Frank Herbert", 1965, true)
    println("dune: $dune")

    val copy = dune.copy()
    println("copy == dune: ${copy == dune}")

    // copy makes a new Book with the properties it names changed.
    val reissue = dune.copy(year = 1966)
    println("reissue: $reissue")
    println("reissue == dune: ${reissue == dune}")
    println("dune < reissue: ${bookOrder.compare(dune, reissue) < 0}")

    println("distinct books: ${setOf(dune, copy, reissue).size}")
}
