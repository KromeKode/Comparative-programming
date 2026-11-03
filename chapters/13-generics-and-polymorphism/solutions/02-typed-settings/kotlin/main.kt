class SettingError(message: String) : Exception(message)

// One setting whose value is a T. It knows how to read a T from text and how
// to show one.
class Setting<T : Any>(
    val name: String,
    private val default: T,
    private val parse: (String) -> T?,
    private val show: (T) -> String,
    private val needs: String,
) {
    var value: T = default
        private set

    fun setText(text: String) {
        value = parse(text) ?: throw SettingError("$name needs $needs, not \"$text\"")
    }

    fun reset() {
        value = default
    }

    fun describe() = "$name = ${show(value)}"

    fun isDefault() = value == default
}

val whole = Regex("-?[0-9]+")

// toIntOrNull would also take "+5", so check the form first. It returns null
// for a number that doesn't fit in an Int.
fun parseInt(text: String): Int? = if (whole.matches(text)) text.toIntOrNull() else null

fun parseBool(text: String): Boolean? =
    when (text) {
        "true" -> true
        "false" -> false
        else -> null
    }

class Settings {
    // Typed properties: volume.value is an Int and muted.value a Boolean.
    val volume = Setting("volume", 5, ::parseInt, Int::toString, "a whole number")
    val muted = Setting("muted", false, ::parseBool, Boolean::toString, "true or false")
    val name = Setting("name", "guest", { it }, { it }, "")
    val tags =
        Setting(
            "tags",
            emptyList(),
            { it.split(" ").filter(String::isNotEmpty) },
            { it.joinToString(", ", "[", "]") },
            "",
        )

    // Setting<*> is a Setting of some type we don't know. Its methods that
    // don't take a T, which is all of them here, can still be called.
    val all: List<Setting<*>> = listOf(volume, muted, name, tags)

    fun find(wanted: String) =
        all.find { it.name == wanted } ?: throw SettingError("no setting named \"$wanted\"")

    fun summary() =
        if (muted.value) "${name.value} has muted the sound"
        else "${name.value} hears the sound at ${volume.value.toLong() * 10}%"
}

fun run(settings: Settings, words: List<String>) {
    when (words.firstOrNull()) {
        "set" -> {
            val setting = settings.find(words[1])
            setting.setText(words.drop(2).joinToString(" "))
            println(setting.describe())
        }
        "get" -> println(settings.find(words[1]).describe())
        "reset" -> {
            val setting = settings.find(words[1])
            setting.reset()
            println(setting.describe())
        }
        "list" ->
            for (setting in settings.all) {
                println(setting.describe() + if (setting.isDefault()) " (default)" else "")
            }
        "summary" -> println(settings.summary())
    }
}

fun main() {
    val settings = Settings()
    for (line in generateSequence(::readlnOrNull)) {
        try {
            run(settings, line.split(" ").filter { it.isNotEmpty() })
        } catch (error: SettingError) {
            println("error: ${error.message}")
        }
    }
}
