// Prints a report of each student's scores. ../README.md says what it should print.
import java.io.File

/** Gives total / count, rounded to the nearest whole number, with halves up. */
fun roundedAverage(total: Int, count: Int): Int = (2 * total + count) / (2 * count)

fun main(args: Array<String>) {
    // Each student's scores, in the order the students first appear.
    val scores = mutableMapOf<String, MutableList<Int>>()
    // The input is the file named on the command line, if there is one, so that a
    // debugger can keep the terminal to itself.
    val lines =
        if (args.isNotEmpty()) File(args[0]).readLines()
        else generateSequence(::readlnOrNull).toList()
    for (line in lines) {
        val fields = line.split(' ', '\t').filter { it.isNotEmpty() }
        if (fields.isEmpty()) continue
        val name = fields[0]
        val marks = fields.drop(1).map { it.toInt() }
        scores.getOrPut(name) { mutableListOf() } += marks
    }

    var bestName = "-"
    var bestAverage = -1
    var classTotal = 0
    var classCount = 0
    for ((name, marks) in scores) {
        var total = 0
        for (mark in marks) {
            total += mark
        }
        classTotal += total
        classCount += marks.size
        if (marks.isEmpty()) {
            println("$name -")
            continue
        }
        val average = roundedAverage(total, marks.size)
        println("$name $average")
        if (average > bestAverage) {
            bestName = name
            bestAverage = average
        }
    }

    if (classCount == 0) {
        println("class average: -")
    } else {
        println("class average: ${roundedAverage(classTotal, classCount)}")
    }
    println("top: $bestName")
}
