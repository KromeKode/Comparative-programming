// Removes repeated lines from each file named on the command line, keeping the first of
// each, and rewrites the file in place. It writes a new file next to the old one and
// renames it over the old one, so that a crash halfway leaves the old file whole.
import java.io.File
import java.io.FileOutputStream
import java.io.IOException
import java.nio.file.Files
import java.nio.file.StandardCopyOption
import kotlin.system.exitProcess

// Rewrites the file without its repeated lines, and returns how many there were.
fun dedup(path: String): Int {
    val file = File(path)
    // readText decodes UTF-8; bytes that aren't UTF-8 become U+FFFD, without an error.
    // split, not lines(), which would also split at a "\r".
    val lines = file.readText().split("\n").toMutableList()
    if (lines.last() == "") lines.removeLast() // it ended with a newline, or was empty
    val kept = lines.distinct() // the first of each, in order
    val removed = lines.size - kept.size
    if (removed == 0) return 0

    val target = file.toPath()
    val folder = target.toAbsolutePath().parent
    // A new file in the same folder, with a fresh name, readable by its owner only.
    val temporary = Files.createTempFile(folder, ".dedup-", null)
    try {
        FileOutputStream(temporary.toFile()).use { out ->
            out.write(kept.joinToString("") { "$it\n" }.toByteArray())
            out.fd.sync() // onto the disk before the rename
        } // use closes it, even if something threw
        Files.setPosixFilePermissions(temporary, Files.getPosixFilePermissions(target))
        Files.move(temporary, target, StandardCopyOption.ATOMIC_MOVE)
    } catch (error: Exception) {
        Files.deleteIfExists(temporary)
        throw error
    }
    return removed
}

fun main(args: Array<String>) {
    if (args.isEmpty()) {
        System.err.println("usage: dedup FILE...")
        exitProcess(2)
    }
    var status = 0
    for (path in args) {
        val removed =
            try {
                dedup(path)
            } catch (error: IOException) {
                // Such as "notes.txt (No such file or directory)", for File's errors.
                System.err.println("dedup: ${error.message}")
                status = 1
                continue
            }
        println("$path: removed $removed line${if (removed == 1) "" else "s"}")
    }
    exitProcess(status)
}
