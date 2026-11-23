// Prints the lines that contain a pattern, from each file named on the command line, or
// from stdin. ../README.md says what it should print.
import java.io.BufferedOutputStream
import java.io.IOException
import java.io.OutputStream
import java.nio.file.Files
import java.nio.file.NoSuchFileException
import java.nio.file.Path
import kotlin.system.exitProcess

const val USAGE = "usage: search [-icnv] PATTERN [FILE...]"
const val STDIN_NAME = "(standard input)"

fun usageError(message: String): Nothing {
    System.err.println("search: $message\n$USAGE")
    exitProcess(2)
}

// What strerror says for the errors the README names. Java's NoSuchFileException has
// only the path as its message.
fun message(error: IOException): String =
    when (error) {
        is NoSuchFileException -> "No such file or directory"
        else -> error.message ?: error.toString() // "Is a directory", for a folder
    }

// A copy of the bytes with A to Z made a to z. Kotlin's Byte is signed, from -128 to
// 127, so the bytes of "é" are negative, and stay as they are.
fun lower(bytes: ByteArray): ByteArray =
    ByteArray(bytes.size) { i ->
        val byte = bytes[i]
        if (byte in 'A'.code..'Z'.code) (byte + ('a' - 'A')).toByte() else byte
    }

// Whether pattern is somewhere in bytes, from start to end.
fun contains(bytes: ByteArray, start: Int, end: Int, pattern: ByteArray): Boolean =
    (start..end - pattern.size).any { i ->
        pattern.indices.all { j -> bytes[i + j] == pattern[j] }
    }

// Writes the lines of data that are selected, and returns how many there are.
fun search(
    out: OutputStream,
    data: ByteArray,
    name: String?,
    pattern: ByteArray,
    options: Set<Char>,
): Int {
    val text = if ('i' in options) lower(data) else data // the bytes to search in
    val prefix = if (name == null) "" else "$name:"
    var count = 0
    var number = 0
    var start = 0
    while (start < data.size) {
        var end = start
        while (end < data.size && data[end] != '\n'.code.toByte()) end++
        number++
        val found = contains(text, start, end, pattern)
        if (found != 'v' in options) {
            count++
            if ('c' !in options) {
                out.write((prefix + if ('n' in options) "$number:" else "").toByteArray())
                out.write(data, start, end - start)
                out.write('\n'.code)
            }
        }
        start = end + 1
    }
    if ('c' in options) out.write("$prefix$count\n".toByteArray())
    return count
}

fun main(arguments: Array<String>) {
    val args = arguments.toMutableList()
    val options = mutableSetOf<Char>()
    // Options come first; the first argument that isn't one is the pattern.
    while (args.isNotEmpty() && args[0].startsWith("-") && args[0] != "-") {
        val arg = args.removeFirst()
        if (arg == "--") break
        for (letter in arg.drop(1)) {
            if (letter !in "icnv") usageError("unknown option -$letter")
            options.add(letter)
        }
    }
    if (args.isEmpty()) usageError("no pattern")
    val pattern = args[0].toByteArray().let { if ('i' in options) lower(it) else it }
    val files = args.drop(1).ifEmpty { listOf("-") }

    // System.out flushes at every newline; this saves the bytes up instead.
    val out = BufferedOutputStream(System.out)
    var selected = 0
    var status = 0
    for (file in files) {
        val data =
            try {
                if (file == "-") System.`in`.readBytes() else Files.readAllBytes(Path.of(file))
            } catch (error: IOException) {
                out.flush() // what came before, before the error
                System.err.println("search: $file: ${message(error)}")
                status = 2
                continue
            }
        val name = if (files.size == 1) null else if (file == "-") STDIN_NAME else file
        selected += search(out, data, name, pattern, options)
    }
    out.flush()
    exitProcess(if (status == 0 && selected == 0) 1 else status)
}
