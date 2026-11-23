// Reads a trace of function calls and prints a flat profile: for each function, how
// many calls it got, its total time, and its self time. See ../README.md.

class FunctionStats {
    var calls = 0L
    var total = 0L
    var self = 0L
    var running = 0L // how many of its calls are running now
    var outermostStart = 0L // when the outermost of them started
}

class Call(val name: String, val start: Long) {
    var inCallees = 0L // time spent in the functions it called
}

class TraceError(message: String) : Exception(message)

val timePattern = Regex("[0-9]{1,12}")
val namePattern = Regex("[!-~]+")

fun profile(lines: Sequence<String>): Map<String, FunctionStats> {
    val functions = mutableMapOf<String, FunctionStats>()
    val stack = ArrayDeque<Call>()
    var now = 0L
    for ((index, line) in lines.withIndex()) {
        val number = index + 1
        val fields = line.split(' ', '\t').filter { it.isNotEmpty() }
        if (fields.isEmpty()) continue
        if (
            fields.size != 3 ||
                !timePattern.matches(fields[0]) ||
                !namePattern.matches(fields[2]) ||
                fields[1] !in listOf("enter", "exit")
        ) {
            throw TraceError("line $number: expected TIME enter NAME or TIME exit NAME")
        }
        val time = fields[0].toLong()
        val name = fields[2]
        if (time < now) throw TraceError("line $number: time goes back from $now to $time")
        now = time

        if (fields[1] == "enter") {
            val f = functions.getOrPut(name) { FunctionStats() }
            f.calls++
            if (f.running == 0L) f.outermostStart = time
            f.running++
            stack.addLast(Call(name, time))
            continue
        }

        val call =
            stack.lastOrNull()
                ?: throw TraceError("line $number: exit from $name, but nothing is running")
        if (call.name != name) {
            throw TraceError(
                "line $number: exit from $name, but the innermost call is to ${call.name}"
            )
        }
        stack.removeLast()
        val duration = time - call.start
        val f = functions.getValue(name)
        f.self += duration - call.inCallees
        f.running--
        // A recursive call's time is already in the total of the call around it.
        if (f.running == 0L) f.total += time - f.outermostStart
        stack.lastOrNull()?.let { it.inCallees += duration }
    }
    stack.lastOrNull()?.let { throw TraceError("end of trace: ${it.name} is still running") }
    return functions
}

fun main() {
    val functions =
        try {
            profile(generateSequence(::readlnOrNull))
        } catch (error: TraceError) {
            println("error: ${error.message}")
            return
        }
    if (functions.isEmpty()) println("no calls")
    val rows = functions.entries.sortedWith(compareBy({ -it.value.self }, { it.key }))
    for ((name, f) in rows) println("$name ${f.calls} ${f.total} ${f.self}")
}
