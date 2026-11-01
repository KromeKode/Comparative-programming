const val MAX_OPEN = 2
const val MAX_LINES = 8

class DiskError(message: String) : Exception(message)

class FileData {
    val lines = mutableListOf<String>()
    var locked = false
}

class Disk {
    val files = mutableMapOf<String, FileData>()
    var openCount = 0

    fun linesUsed() = files.values.sumOf { it.lines.size }

    // Returns an open Handle. It is AutoCloseable, so `use` can close it.
    fun open(name: String, writing: Boolean = false): Handle {
        if (openCount == MAX_OPEN) throw DiskError("too many open files")
        val file = files[name]
        if (writing) {
            if (file != null && file.locked) throw DiskError("\"$name\" is read-only")
            files.getOrPut(name) { FileData() }.lines.clear()
        } else if (file == null) {
            throw DiskError("no file \"$name\"")
        }
        openCount++
        println("open $name")
        return Handle(this, name)
    }
}

class Handle(private val disk: Disk, private val name: String) : AutoCloseable {
    private val file
        get() = disk.files.getValue(name)

    fun lines(): List<String> = file.lines.toList()

    fun write(line: String) {
        if (disk.linesUsed() == MAX_LINES) throw DiskError("disk full")
        file.lines.add(line)
    }

    override fun close() {
        disk.openCount--
        println("close $name")
    }
}

fun lines(count: Int) = if (count == 1) "1 line" else "$count lines"

fun writeFile(disk: Disk, name: String, words: List<String>) {
    disk.open(name, writing = true).use { target -> words.forEach(target::write) }
    println("wrote ${lines(words.size)}")
}

fun catFile(disk: Disk, name: String) {
    disk.open(name).use { source -> source.lines().forEach { println("  $it") } }
}

fun copyFile(disk: Disk, from: String, to: String) {
    if (from == to) throw DiskError("cannot copy \"$from\" onto itself")
    var copied = 0
    disk.open(from).use { source ->
        disk.open(to, writing = true).use { target ->
            for (line in source.lines()) {
                target.write(line)
                copied++
            }
        }
    }
    println("copied ${lines(copied)}")
}

fun lockFile(disk: Disk, name: String) {
    val file = disk.files[name] ?: throw DiskError("no file \"$name\"")
    file.locked = true
    println("locked $name")
}

fun takes(command: String, want: Int, args: List<String>) {
    if (args.size != want) {
        val noun = if (want == 1) "name" else "names"
        throw DiskError("$command takes $want $noun, got ${args.size}")
    }
}

fun run(disk: Disk, command: String, args: List<String>) {
    when (command) {
        "write" -> {
            if (args.isEmpty()) throw DiskError("write needs a file name")
            writeFile(disk, args[0], args.drop(1))
        }
        "cat" -> {
            takes(command, 1, args)
            catFile(disk, args[0])
        }
        "copy" -> {
            takes(command, 2, args)
            copyFile(disk, args[0], args[1])
        }
        "lock" -> {
            takes(command, 1, args)
            lockFile(disk, args[0])
        }
        else -> throw DiskError("unknown command \"$command\"")
    }
}

fun main() {
    val disk = Disk()
    for (line in generateSequence(::readlnOrNull)) {
        val parts = line.split(Regex("\\s+")).filter { it.isNotEmpty() }
        if (parts.isEmpty()) continue
        try {
            run(disk, parts[0], parts.drop(1))
        } catch (error: DiskError) {
            println("error: ${error.message}")
        }
    }
    println("files still open: ${disk.openCount}")
}
