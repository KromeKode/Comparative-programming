import java.util.concurrent.ArrayBlockingQueue
import kotlin.concurrent.thread

const val COUNT = 1000L
const val WORKERS = 3

// Java's queues can't be closed, so a special value says "stop".
const val STOP = -1L

fun main() {
    // A BlockingQueue is safe to use from several threads. put waits while
    // the queue is full, and take waits while it is empty.
    val jobs = ArrayBlockingQueue<Long>(16)
    val results = ArrayBlockingQueue<Long>(16)

    thread {
        for (n in 1..COUNT) jobs.put(n)
        repeat(WORKERS) { jobs.put(STOP) } // one for each worker
    }

    repeat(WORKERS) {
        thread {
            while (true) {
                val n = jobs.take()
                if (n == STOP) break
                results.put(n * n)
            }
            results.put(STOP) // this worker is done
        }
    }

    var received = 0
    var sum = 0L
    var finished = 0
    while (finished < WORKERS) {
        val square = results.take()
        if (square == STOP) {
            finished++
        } else {
            received++
            sum += square
        }
    }
    println("$WORKERS workers squared 1 to $COUNT")
    println("results: $received")
    println("sum: $sum")
}
