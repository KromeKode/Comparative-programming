import kotlin.reflect.KFunction

object Tracer {
    var on = false
    var depth = 0
    var calls = 0

    fun say(text: String) {
        if (on) println("  ".repeat(depth) + text)
    }
}

// traced runs body as one traced call. ::fib is a reference to the function
// fib, and its name property is "fib".
inline fun traced(function: KFunction<Long>, vararg args: Long, body: () -> Long): Long {
    Tracer.calls++
    Tracer.say("${function.name}(${args.joinToString(", ")})")
    Tracer.depth++
    val result =
        try {
            body()
        } finally {
            Tracer.depth--
        }
    Tracer.say("-> $result")
    return result
}

// The recursive calls in each body go through the function, and so through
// traced, again.
fun fib(n: Long): Long = traced(::fib, n) { if (n < 2) n else fib(n - 1) + fib(n - 2) }

fun gcd(a: Long, b: Long): Long = traced(::gcd, a, b) { if (b == 0L) a else gcd(b, a % b) }

fun power(base: Long, exponent: Long): Long =
    traced(::power, base, exponent) {
        when {
            exponent == 0L -> 1
            exponent % 2 == 0L -> power(base, exponent / 2).let { it * it }
            else -> base * power(base, exponent - 1)
        }
    }

// Reflection, from kotlin-reflect, gives each function's name and parameters,
// and call runs a function with an array of arguments.
val functions = listOf(::fib, ::gcd, ::power).associateBy { it.name }

fun plural(count: Int, word: String) = "$count $word" + if (count == 1) "" else "s"

fun run(line: String) {
    val words = line.trim().split(Regex("\\s+"))
    val (command, name) = words
    val args = words.drop(2).map { it.toLong() }
    if (command != "trace" && command != "count") {
        println("unknown command: $command")
        return
    }
    val function = functions[name]
    if (function == null) {
        println("unknown function: $name")
        return
    }
    val arity = function.parameters.size
    if (args.size != arity) {
        println("$name takes ${plural(arity, "argument")}, got ${args.size}")
        return
    }
    Tracer.on = command == "trace"
    Tracer.depth = 0
    Tracer.calls = 0
    val result = function.call(*args.toTypedArray())
    println("$name(${args.joinToString(", ")}) = $result in ${plural(Tracer.calls, "call")}")
}

fun main() {
    for (line in generateSequence(::readlnOrNull)) {
        if (line.isNotBlank()) run(line)
    }
}
