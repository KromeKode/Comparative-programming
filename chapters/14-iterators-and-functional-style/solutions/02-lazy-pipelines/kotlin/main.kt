fun isPrime(n: Long): Boolean =
    n >= 2 && generateSequence(2L) { it + 1 }.takeWhile { it * it <= n }.none { n % it == 0L }

// Sequences are lazy, so the endless sources cost nothing until asked.
fun naturals(): Sequence<Long> = generateSequence(1L) { it + 1 }

fun fibonacci(): Sequence<Long> =
    generateSequence(0L to 1L) { (a, b) -> b to a + b }.map { it.first }

fun source(words: List<String>): Sequence<Long> =
    when (words[0]) {
        "naturals" -> naturals()
        "primes" -> naturals().filter(::isPrime)
        "fibonacci" -> fibonacci()
        "range" -> (words[1].toLong()..words[2].toLong()).asSequence()
        "repeat" -> words[1].toLong().let { n -> generateSequence { n } }
        else -> error("unknown source $words")
    }

fun predicate(words: List<String>): (Long) -> Boolean =
    when (words[0]) {
        "even" -> { x ->
            x % 2 == 0L
        }
        "odd" -> { x ->
            x % 2 != 0L
        }
        "prime" -> ::isPrime
        "above" -> words[1].toLong().let { n -> { x -> x > n } }
        "below" -> words[1].toLong().let { n -> { x -> x < n } }
        else -> error("unknown predicate $words")
    }

fun function(words: List<String>): (Long) -> Long =
    when (words[0]) {
        "square" -> { x ->
            x * x
        }
        "negate" -> { x ->
            -x
        }
        "add" -> words[1].toLong().let { n -> { x -> x + n } }
        "mul" -> words[1].toLong().let { n -> { x -> x * n } }
        else -> error("unknown function $words")
    }

// Each stage wraps the sequence it is given in a new lazy one.
fun stage(items: Sequence<Long>, words: List<String>): Sequence<Long> {
    val rest = words.drop(1)
    return when (words[0]) {
        "map" -> items.map(function(rest))
        "filter" -> items.filter(predicate(rest))
        "takewhile" -> items.takeWhile(predicate(rest))
        "dropwhile" -> items.dropWhile(predicate(rest))
        "take" -> items.take(rest[0].toInt())
        "skip" -> items.drop(rest[0].toInt())
        "step" -> rest[0].toInt().let { n -> items.filterIndexed { i, _ -> i % n == 0 } }
        else -> error("unknown stage $words")
    }
}

fun run(line: String): String {
    val parts = line.split("|").map { it.trim().split(Regex("\\s+")) }
    // A sum or count at the end is not a stage: it says what to do with the items.
    val finish = parts.last().singleOrNull()?.takeIf { it == "sum" || it == "count" }
    val stages = parts.drop(1).dropLast(if (finish == null) 0 else 1)
    val items = stages.fold(source(parts[0]), ::stage)
    // Nothing has run yet. Only the lines below pull items through.
    return when (finish) {
        "sum" -> items.sum().toString()
        "count" -> items.count().toString()
        else -> items.joinToString(" ").ifEmpty { "(empty)" }
    }
}

fun main() {
    for (line in generateSequence(::readlnOrNull)) {
        if (line.isNotBlank()) println(run(line))
    }
}
