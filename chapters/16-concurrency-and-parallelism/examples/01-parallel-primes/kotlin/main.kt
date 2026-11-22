import java.util.concurrent.Callable
import java.util.concurrent.Executors

const val LIMIT = 200_000
const val WORKERS = 4

fun isPrime(n: Int): Boolean {
    if (n < 2) return false
    var d = 2
    while (d * d <= n) {
        if (n % d == 0) return false
        d++
    }
    return true
}

fun countPrimes(start: Int, end: Int) = (start until end).count(::isPrime)

fun main() {
    println("counting primes below $LIMIT with $WORKERS workers")
    val jobs = (0 until WORKERS).map { i -> LIMIT * i / WORKERS to LIMIT * (i + 1) / WORKERS }

    // A thread pool runs the tasks it is given on its own threads. submit
    // returns a Future, get waits for its result, and use closes the pool,
    // which waits for every task to finish.
    val counts =
        Executors.newFixedThreadPool(WORKERS).use { pool ->
            jobs
                .map { (start, end) -> pool.submit(Callable { countPrimes(start, end) }) }
                .map { it.get() }
        }
    for ((i, job) in jobs.withIndex()) {
        println("worker $i: [${job.first}, ${job.second}) has ${counts[i]} primes")
    }
    println("total: ${counts.sum()} primes")
}
