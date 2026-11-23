// Reads a web server's log, one request on each line, such as
//   2026-11-23 14:05:12 GET /books/42 200 12ms
// and prints, for each method and route, how many requests there were, how many failed,
// and the slowest. A route is the path with each "/" and the digits after it written
// "/:id", so that /books/42 and /books/7 count together. A line that isn't a request
// gets a message on stderr, and the exit status is then 1.
import kotlin.system.exitProcess

// matchEntire matches the whole line, so the pattern needs no ^ or $. In Kotlin on the
// JVM, \d is only [0-9], but this says so.
val LINE =
    Regex(
        """(?<date>[0-9]{4}-[0-9]{2}-[0-9]{2}) (?<time>[0-9]{2}:[0-9]{2}:[0-9]{2})""" +
            """ (?<method>GET|POST|PUT|DELETE) (?<path>/[^ ]*)""" +
            """ (?<status>[0-9]{3}) (?<ms>[0-9]{1,9})ms"""
    )
val ID = Regex("/[0-9]+")

class Route {
    var requests = 0
    var failed = 0
    var slowest = 0
}

fun main() {
    // A LinkedHashMap keeps its keys in the order they went in.
    val routes = LinkedHashMap<String, Route>()
    var status = 0
    // A BufferedReader's lines would also split at a "\r" on its own, so this splits at
    // "\n" only.
    val lines = System.`in`.bufferedReader().readText().split("\n").toMutableList()
    // The last is empty when the input ended with a newline, or was empty.
    if (lines.last() == "") lines.removeLast()
    for ((index, line) in lines.withIndex()) {
        val match = LINE.matchEntire(line.removeSuffix("\r"))
        if (match == null) {
            System.err.println("logscan: line ${index + 1}: not a log line")
            status = 1
            continue
        }
        // groups[name] is null only for a group that didn't take part in the match.
        fun group(name: String) = match.groups[name]!!.value
        // In a replacement, "$" and "\" are special; "/:id" has neither.
        val key = group("method") + " " + ID.replace(group("path"), "/:id")
        val route = routes.getOrPut(key) { Route() }
        route.requests++
        if (group("status").toInt() >= 400) route.failed++
        route.slowest = maxOf(route.slowest, group("ms").toInt())
    }
    for ((key, route) in routes) {
        val plural = if (route.requests == 1) "" else "s"
        println(
            "$key: ${route.requests} request$plural, ${route.failed} failed, " +
                "slowest ${route.slowest}ms"
        )
    }
    exitProcess(status)
}
