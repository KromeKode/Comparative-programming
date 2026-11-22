import java.util.PriorityQueue

// A sealed interface has only the subtypes in this file, so a when over a
// Step can check that it covers every kind.
sealed interface Step

data class Print(val text: String) : Step

data class Sleep(val duration: Long) : Step

data class Wait(val task: Int) : Step // the number of the task to wait for

class Task(val name: String) {
    val steps = mutableListOf<Step>()
    var next = 0 // the index of the step to run when it runs again
    var done = false
    var waitingFor = 0
    val waiters = mutableListOf<Int>() // in the order they began waiting
}

// The first word of a line, and the rest of it.
fun split(line: String): Pair<String, String> {
    val trimmed = line.trim()
    return trimmed.substringBefore(' ') to trimmed.substringAfter(' ', "").trim()
}

// Returns the tasks, and prints an error for each line with a problem.
fun parse(lines: List<String>): List<Task> {
    // A wait can name a task defined further down, so the names come first.
    val numbers = mutableMapOf<String, Int>()
    for (line in lines) {
        val (command, name) = split(line)
        if (command == "task") numbers.getOrPut(name) { numbers.size }
    }
    val tasks = mutableListOf<Task>()
    for ((i, line) in lines.withIndex()) {
        val (command, rest) = split(line)
        fun error(message: String) = println("error: line ${i + 1}: $message")
        val steps = tasks.lastOrNull()?.steps
        val other = numbers[rest]
        when {
            command.isEmpty() -> {}
            command == "task" -> tasks.add(Task(rest))
            steps == null -> error("a step before any task")
            command == "print" -> steps.add(Print(rest))
            command == "sleep" && rest.isNotEmpty() && rest.all { it in '0'..'9' } ->
                steps.add(Sleep(rest.toLong()))
            command == "sleep" -> error("sleep takes a whole number")
            command == "wait" && other != null -> steps.add(Wait(other))
            command == "wait" -> error("no task named $rest")
            else -> error("unknown step: $command")
        }
    }
    return tasks
}

data class Sleeper(val wake: Long, val order: Long, val task: Int)

class Loop(private val tasks: List<Task>) {
    private var time = 0L
    private val ready = ArrayDeque(tasks.indices.toList()) // the tasks that can run now, in order
    // A PriorityQueue's head is its smallest item, by the comparator.
    private val sleeping = PriorityQueue(compareBy<Sleeper>({ it.wake }, { it.order }))
    private var sleeps = 0L

    // Runs a task until it sleeps, waits, or ends.
    private fun runUntilStopped(id: Int) {
        val task = tasks[id]
        while (task.next < task.steps.size) {
            when (val step = task.steps[task.next++]) {
                is Print -> println("[$time] ${task.name}: ${step.text}")
                is Sleep -> {
                    if (step.duration == 0L) ready.addLast(id)
                    else sleeping.add(Sleeper(time + step.duration, sleeps++, id))
                    return
                }
                is Wait -> {
                    if (tasks[step.task].done) continue // it has finished already, so carry on
                    task.waitingFor = step.task
                    tasks[step.task].waiters.add(id)
                    return
                }
            }
        }
        task.done = true
        println("[$time] ${task.name}: done")
        ready.addAll(task.waiters)
    }

    fun run() {
        while (true) {
            while (ready.isNotEmpty()) runUntilStopped(ready.removeFirst())
            // Nothing can run now, so the clock jumps to the next wake.
            time = sleeping.peek()?.wake ?: break
            while (sleeping.peek()?.wake == time) ready.addLast(sleeping.poll().task)
        }
        val stuck =
            tasks.filter { !it.done }.map { "${it.name} waits for ${tasks[it.waitingFor].name}" }
        if (stuck.isNotEmpty()) println("[$time] deadlock: ${stuck.joinToString(", ")}")
        else println("[$time] all done")
    }
}

fun main() {
    Loop(parse(generateSequence(::readlnOrNull).toList())).run()
}
