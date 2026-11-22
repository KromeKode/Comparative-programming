import kotlin.coroutines.Continuation
import kotlin.coroutines.EmptyCoroutineContext
import kotlin.coroutines.createCoroutine
import kotlin.coroutines.resume
import kotlin.coroutines.suspendCoroutine

// Kotlin has suspend functions, and its standard library has the pieces to
// start and resume them, but no event loop. The kotlinx.coroutines library
// supplies one, with launch, async, and delay. This is a small one, to show
// what such a loop does.

// The coroutines that are ready to run, in the order they will run.
val ready = ArrayDeque<Continuation<Unit>>()

// Suspends the coroutine, and puts the rest of it at the back of the line.
suspend fun pause() = suspendCoroutine<Unit> { rest -> ready.addLast(rest) }

// Runs the tasks on this one thread until all of them finish, and returns
// their results in order.
fun <T> runAll(tasks: List<suspend () -> T>): List<T> {
    val results = MutableList<T?>(tasks.size) { null }
    for ((i, task) in tasks.withIndex()) {
        // createCoroutine makes a coroutine that hasn't started. The
        // Continuation passed to it gets the result when it finishes.
        val start =
            task.createCoroutine(
                Continuation(EmptyCoroutineContext) { result ->
                    results[i] = result.getOrThrow()
                }
            )
        ready.addLast(start)
    }
    while (ready.isNotEmpty()) {
        ready.removeFirst().resume(Unit) // runs the coroutine up to its next pause
    }
    return results.map { it!! }
}

suspend fun task(name: String, steps: Int): Int {
    for (step in 1..steps) {
        println("$name: step $step")
        // Calling a suspend function is the only place a task can stop and
        // let another one run. Kotlin has no await keyword.
        pause()
    }
    println("$name: done")
    return steps
}

class Counter {
    var value = 0
}

suspend fun addAtOnce(count: Counter) {
    count.value += 1 // no suspend call, so no other task can run in the middle
}

suspend fun addWithAwait(count: Counter) {
    val value = count.value
    pause() // every other task reads the old value here
    count.value = value + 1
}

fun main() {
    println("three tasks, each pausing at every await")
    val results =
        runAll(listOf(suspend { task("a", 2) }, suspend { task("b", 3) }, suspend { task("c", 1) }))
    println("results: ${results.joinToString(", ")}")

    println("4 tasks add 1 to a shared count")
    val atOnce = Counter()
    runAll(List(4) { suspend { addAtOnce(atOnce) } })
    println("with no await between reading and writing: ${atOnce.value}")
    val withAwait = Counter()
    runAll(List(4) { suspend { addWithAwait(withAwait) } })
    println("with an await between reading and writing: ${withAwait.value}")
}
