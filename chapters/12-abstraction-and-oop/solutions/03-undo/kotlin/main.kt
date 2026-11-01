class EditError(message: String) : Exception(message)

// An edit that knows how to do itself and how to undo itself.
interface Command {
    fun apply(lines: MutableList<String>)

    fun undo(lines: MutableList<String>)

    fun describe(): String
}

fun checkLine(lines: List<String>, number: Int) {
    if (number !in 1..lines.size) throw EditError("no line $number")
}

class Add(private val text: String) : Command {
    override fun apply(lines: MutableList<String>) {
        lines.add(text)
    }

    override fun undo(lines: MutableList<String>) {
        lines.removeLast()
    }

    override fun describe() = "add \"$text\""
}

class Delete(private val number: Int) : Command {
    private var removed = ""

    override fun apply(lines: MutableList<String>) {
        checkLine(lines, number)
        removed = lines.removeAt(number - 1)
    }

    override fun undo(lines: MutableList<String>) {
        lines.add(number - 1, removed)
    }

    override fun describe() = "delete line $number"
}

class Replace(private val number: Int, private val text: String) : Command {
    private var old = ""

    override fun apply(lines: MutableList<String>) {
        checkLine(lines, number)
        old = lines.set(number - 1, text)
    }

    override fun undo(lines: MutableList<String>) {
        lines[number - 1] = old
    }

    override fun describe() = "replace line $number with \"$text\""
}

class Editor {
    private val lines = mutableListOf<String>()
    private val done = ArrayDeque<Command>()
    private val undone = ArrayDeque<Command>()

    fun run(command: Command) {
        command.apply(lines) // if this throws, nothing is recorded
        done.addLast(command)
        undone.clear()
    }

    fun undo() {
        val command = done.removeLastOrNull() ?: throw EditError("nothing to undo")
        command.undo(lines)
        undone.addLast(command)
        println("undo: ${command.describe()}")
    }

    fun redo() {
        val command = undone.removeLastOrNull() ?: throw EditError("nothing to redo")
        command.apply(lines)
        done.addLast(command)
        println("redo: ${command.describe()}")
    }

    fun show() {
        if (lines.isEmpty()) println("(empty)")
        lines.forEachIndexed { i, line -> println("${i + 1}: $line") }
    }
}

fun parseNumber(text: String) = text.toIntOrNull() ?: throw EditError("not a line number: $text")

fun run(editor: Editor, words: List<String>) {
    val command = words.firstOrNull() ?: return
    val args = words.drop(1)
    when {
        command == "add" -> editor.run(Add(args.joinToString(" ")))
        command == "delete" && args.size == 1 -> editor.run(Delete(parseNumber(args[0])))
        command == "replace" && args.isNotEmpty() ->
            editor.run(Replace(parseNumber(args[0]), args.drop(1).joinToString(" ")))
        command == "undo" -> editor.undo()
        command == "redo" -> editor.redo()
        command == "show" -> editor.show()
    }
}

fun main() {
    val editor = Editor()
    for (line in generateSequence(::readlnOrNull)) {
        try {
            run(editor, line.split(Regex("\\s+")).filter { it.isNotEmpty() })
        } catch (error: EditError) {
            println("error: ${error.message}")
        }
    }
}
