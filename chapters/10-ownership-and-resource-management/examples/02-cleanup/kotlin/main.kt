import java.io.IOException

// A pretend file. AutoCloseable lets `use` close it.
class File(val name: String, fails: Boolean = false) : AutoCloseable {
    init {
        if (fails) throw IOException("cannot open $name")
        println("  open $name")
    }

    override fun close() = println("  close $name")
}

// use runs its block, then closes the file however the block ended.
fun runJob(outputFails: Boolean, copyFails: Boolean) {
    File("input").use {
        File("output", outputFails).use {
            if (copyFails) throw IOException("disk full")
            println("  copy input to output")
        }
    }
}

fun main() {
    val jobs =
        listOf(
            Triple("everything works", false, false),
            Triple("opening output fails", true, false),
            Triple("copying fails", false, true),
        )
    for ((i, job) in jobs.withIndex()) {
        val (title, outputFails, copyFails) = job
        val number = i + 1
        println("job $number: $title")
        try {
            runJob(outputFails, copyFails)
            println("  job $number done")
        } catch (error: IOException) {
            println("  job $number failed: ${error.message}")
        }
    }
}
