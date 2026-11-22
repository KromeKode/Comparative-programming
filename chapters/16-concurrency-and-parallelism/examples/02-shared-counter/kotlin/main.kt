import java.util.concurrent.atomic.AtomicLong
import java.util.concurrent.locks.ReentrantLock
import kotlin.concurrent.thread
import kotlin.concurrent.withLock

const val THREADS = 4
const val TIMES = 100_000

// Starts work on several threads, and waits for all of them.
fun run(work: () -> Unit) {
    val threads = List(THREADS) { thread { work() } }
    threads.forEach { it.join() }
}

fun main() {
    println("$THREADS threads add 1, $TIMES times each")

    // withLock holds the lock while the block runs, and releases it after,
    // even if the block throws.
    var lockedCount = 0L
    val lock = ReentrantLock()
    run { repeat(TIMES) { lock.withLock { lockedCount++ } } }
    println("with a lock: $lockedCount")

    // incrementAndGet reads, adds, and writes as one step.
    val atomicCount = AtomicLong()
    run { repeat(TIMES) { atomicCount.incrementAndGet() } }
    println("with an atomic: $atomicCount")
}
