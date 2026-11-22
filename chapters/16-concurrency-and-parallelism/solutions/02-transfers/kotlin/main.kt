import java.util.concurrent.locks.ReentrantLock
import kotlin.concurrent.thread
import kotlin.concurrent.withLock

data class Transfer(val source: Int, val target: Int, val amount: Long)

fun plural(count: Int, noun: String) = if (count == 1) "$count $noun" else "$count ${noun}s"

// The whole numbers on a line, or null if a word isn't one.
fun numbers(line: String): List<Long>? {
    val words = line.trim().split(Regex("\\s+")).filter { it.isNotEmpty() }
    return if (words.all { it.all { c -> c in '0'..'9' } }) words.map(String::toLong) else null
}

class Bank(initial: List<Long>) {
    val balances = initial.toLongArray()
    // One lock for each account, so that transfers between different
    // accounts can run at the same time.
    private val locks = List(initial.size) { ReentrantLock() }

    fun transfer(t: Transfer) {
        // A ReentrantLock can be taken twice by one thread, but the second
        // write below would undo the first, so this has to return anyway.
        if (t.source == t.target) return
        // Every worker takes the two locks in the same order, lower account
        // first, so no two workers can each hold the lock the other wants.
        locks[minOf(t.source, t.target)].withLock {
            locks[maxOf(t.source, t.target)].withLock {
                val sourceBalance = balances[t.source]
                val targetBalance = balances[t.target]
                // Lets another thread run between the read and the write, as
                // it could while a real bank waits on its database.
                Thread.yield()
                balances[t.source] = sourceBalance - t.amount
                balances[t.target] = targetBalance + t.amount
            }
        }
    }
}

class InputError(message: String) : Exception(message)

fun parse(lines: List<String>): Triple<Int, List<Long>, List<Transfer>> {
    val first = numbers(lines.getOrElse(0) { "" })
    if (first == null || first.size != 1 || first[0] !in 1L..16L) {
        throw InputError("line 1 must be the number of workers, from 1 to 16")
    }
    val balances = numbers(lines.getOrElse(1) { "" })
    if (balances.isNullOrEmpty()) throw InputError("line 2 must be the starting balances")
    val transfers = mutableListOf<Transfer>()
    for (i in 2 until lines.size) {
        val words = numbers(lines[i])
        if (words != null && words.isEmpty()) continue
        if (words == null || words.size != 3) {
            println("error: line ${i + 1}: a transfer is three numbers: from, to, and amount")
            continue
        }
        val (source, target, amount) = words
        val missing = listOf(source, target).firstOrNull { it >= balances.size }
        if (missing != null) {
            println("error: line ${i + 1}: no account $missing")
            continue
        }
        transfers.add(Transfer(source.toInt(), target.toInt(), amount))
    }
    return Triple(first[0].toInt(), balances, transfers)
}

fun main() {
    val (workers, balances, transfers) =
        try {
            parse(generateSequence(::readlnOrNull).toList())
        } catch (error: InputError) {
            println("error: ${error.message}")
            return
        }
    val bank = Bank(balances)

    // Worker i applies transfers i, i + workers, i + 2 * workers, and so on.
    // thread starts a thread at once, and join waits for it to finish.
    val shares = List(workers) { i -> transfers.filterIndexed { j, _ -> j % workers == i } }
    val threads = shares.map { share -> thread { share.forEach(bank::transfer) } }
    threads.forEach(Thread::join)

    val counts =
        listOf(
            plural(workers, "worker"),
            plural(balances.size, "account"),
            plural(transfers.size, "transfer"),
        )
    println(counts.joinToString(", "))
    for ((i, share) in shares.withIndex()) {
        println("worker $i: ${plural(share.size, "transfer")}, ${share.sumOf { it.amount }} moved")
    }
    for ((i, balance) in bank.balances.withIndex()) println("account $i: $balance")
    println("total: ${bank.balances.sum()}")
}
