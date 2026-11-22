// Exercise 18.3: version ranges. The highest version that each range allows.
//
// The versions and ranges are package semver's business; this file reads the input
// and prints the answers.
import semver.Range
import semver.Version
import semver.parseVersion

fun main() {
    val lines = generateSequence(::readlnOrNull).toList()
    if (lines.isEmpty()) return
    val versions = mutableListOf<Version>()
    for (word in lines[0].split(Regex("\\s+")).filter { it.isNotEmpty() }) {
        val version = parseVersion(word)
        if (version == null) {
            println("error: line 1: $word isn't a version")
            return
        }
        versions += version
    }
    if (versions.isEmpty()) {
        println("error: line 1 must list the versions")
        return
    }
    for ((index, raw) in lines.withIndex().drop(1)) {
        val line = raw.trim()
        if (line.isEmpty()) continue
        try {
            val best = Range.parse(line).highest(versions)
            println("$line -> ${best ?: "none"}")
        } catch (error: IllegalArgumentException) {
            println("error: line ${index + 1}: ${error.message}")
        }
    }
}
