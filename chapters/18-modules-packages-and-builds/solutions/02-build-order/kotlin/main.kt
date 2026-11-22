// Exercise 18.2: build order. The order to build modules in, and what to rebuild.
import java.util.PriorityQueue

private val namePattern = Regex("[A-Za-z0-9_./-]+")

private class Module(val line: Int, val imports: List<String>)

private fun words(text: String) = text.split(Regex("\\s+")).filter { it.isNotEmpty() }

private fun run(lines: List<String>): List<String> {
    val out = mutableListOf<String>()
    // mutableMapOf makes a LinkedHashMap, which keeps its keys in the order they were
    // added: the order of definition.
    val modules = mutableMapOf<String, Module>()
    val queries = mutableListOf<Pair<Int, String>>()
    for ((index, line) in lines.withIndex()) {
        val number = index + 1
        val colon = line.indexOf(':')
        if (colon >= 0) {
            val name = line.substring(0, colon).trim()
            val imports = words(line.substring(colon + 1))
            if (!namePattern.matches(name) || !imports.all { namePattern.matches(it) }) {
                out += "error: line $number: a module is NAME: IMPORTS"
                return out
            }
            modules[name]?.let { earlier ->
                out += "error: line $number: $name is already defined on line ${earlier.line}"
                return out
            }
            // distinct drops repeats and keeps the first of each, in order.
            modules[name] = Module(number, imports.distinct())
            continue
        }
        val words = words(line)
        when {
            words.isEmpty() -> {}
            words.size == 2 && words[0] == "rebuild" -> queries += number to words[1]
            else -> {
                out += "error: line $number: expected NAME: IMPORTS or rebuild NAME"
                return out
            }
        }
    }

    val users = modules.keys.associateWith { mutableListOf<String>() }
    val waiting = mutableMapOf<String, Int>()
    for ((name, module) in modules) {
        for (imported in module.imports) {
            if (imported !in modules) {
                out += "error: line ${module.line}: $name imports $imported, which isn't defined"
                return out
            }
            users.getValue(imported) += name
        }
        waiting[name] = module.imports.size
    }

    // Kahn's algorithm: a module is ready once everything it imports is built, and
    // of the ready modules, the first in alphabetical order goes next. Java's
    // PriorityQueue gives the smallest first.
    val ready = PriorityQueue(modules.keys.filter { waiting[it] == 0 })
    val order = mutableListOf<String>()
    while (ready.isNotEmpty()) {
        val name = ready.poll()
        order += name
        for (user in users.getValue(name)) {
            waiting[user] = waiting.getValue(user) - 1
            if (waiting[user] == 0) ready += user
        }
    }

    if (order.size < modules.size) {
        // Every module left over imports another one left over, so following first
        // imports from the first module left over must come back to a module.
        fun isLeft(name: String) = waiting.getValue(name) > 0
        val path = mutableListOf(modules.keys.filter(::isLeft).min())
        val seen = mutableMapOf(path[0] to 0)
        while (true) {
            val next = modules.getValue(path.last()).imports.first(::isLeft)
            seen[next]?.let { at ->
                out += "error: import cycle: " + (path.drop(at) + next).joinToString(" -> ")
                return out
            }
            seen[next] = path.size
            path += next
        }
    }

    out += "build order: " + if (order.isEmpty()) "nothing to build" else order.joinToString(", ")
    for ((number, name) in queries) {
        if (name !in modules) {
            out += "error: line $number: no module called $name"
            continue
        }
        // Everything that imports the changed module, directly or not, comes after it
        // in the build order, so one pass through the order finds them all.
        // mutableSetOf keeps the order things were added in, which here is build order.
        val affected = mutableSetOf(name)
        for (module in order) {
            if (modules.getValue(module).imports.any { it in affected }) affected += module
        }
        out += "rebuild $name: " + affected.joinToString(", ")
    }
    return out
}

fun main() {
    run(generateSequence(::readlnOrNull).toList()).forEach(::println)
}
