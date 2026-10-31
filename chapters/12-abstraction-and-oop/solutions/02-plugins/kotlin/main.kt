class PluginError(message: String) : Exception(message)

// Every plugin has a name and changes a list of words. describe has a body, so
// it is the default for every plugin.
interface Plugin {
    val name: String

    fun apply(words: List<String>): List<String>

    fun describe() = name
}

// A plugin with no settings can be an object: a class with exactly one value.
object Upper : Plugin {
    override val name = "upper"

    override fun apply(words: List<String>) = words.map { it.uppercase() }
}

object Lower : Plugin {
    override val name = "lower"

    override fun apply(words: List<String>) = words.map { it.lowercase() }
}

object Reverse : Plugin {
    override val name = "reverse"

    override fun apply(words: List<String>) = words.reversed()
}

class Censor(private val word: String) : Plugin {
    override val name = "censor"

    override fun apply(words: List<String>) = words.map {
        if (it.equals(word, ignoreCase = true)) "*".repeat(it.length) else it
    }

    override fun describe() = "censor($word)"
}

class Entry(val usage: String, val help: String, val make: (List<String>) -> Plugin)

// Maps a plugin's name to a way to make one. The rest of the program finds
// plugins only through here. mutableMapOf keeps keys in the order added.
class Registry {
    private val entries = mutableMapOf<String, Entry>()

    fun register(name: String, entry: Entry) {
        entries[name] = entry
    }

    fun create(name: String, args: List<String>): Plugin {
        val entry = entries[name] ?: throw PluginError("no plugin named $name")
        return entry.make(args)
    }

    fun entries(): Collection<Entry> = entries.values
}

fun noArguments(plugin: Plugin): (List<String>) -> Plugin = { args ->
    if (args.isNotEmpty()) throw PluginError("${plugin.name} takes no arguments")
    plugin
}

fun makeCensor(args: List<String>): Plugin {
    if (args.size != 1) throw PluginError("censor needs exactly one word")
    return Censor(args[0])
}

fun makeRegistry() =
    Registry().apply {
        register("upper", Entry("upper", "make every letter uppercase", noArguments(Upper)))
        register("lower", Entry("lower", "make every letter lowercase", noArguments(Lower)))
        register(
            "reverse",
            Entry("reverse", "reverse the order of the words", noArguments(Reverse)),
        )
        register("censor", Entry("censor WORD", "hide WORD wherever it appears", ::makeCensor))
    }

fun main() {
    val registry = makeRegistry()
    val pipeline = mutableListOf<Plugin>()
    for (line in generateSequence(::readlnOrNull)) {
        val words = line.split(Regex("\\s+")).filter { it.isNotEmpty() }
        val command = words.firstOrNull() ?: continue
        when {
            command == "list" && words.size == 1 ->
                for (entry in registry.entries()) {
                    println("${entry.usage}: ${entry.help}")
                }
            command == "add" && words.size >= 2 ->
                try {
                    val plugin = registry.create(words[1], words.drop(2))
                    pipeline.add(plugin)
                    println("added ${plugin.describe()}")
                } catch (error: PluginError) {
                    println("error: ${error.message}")
                }
            command == "pipeline" && words.size == 1 -> {
                val steps = pipeline.joinToString(" | ") { it.describe() }
                println("pipeline: ${steps.ifEmpty { "(empty)" }}")
            }
            command == "run" -> {
                var text = words.drop(1)
                for (plugin in pipeline) {
                    text = plugin.apply(text)
                }
                println(text.joinToString(" "))
            }
        }
    }
}
