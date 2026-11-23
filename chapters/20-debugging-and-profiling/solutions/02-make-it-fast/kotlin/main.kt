// Simulates a day of payments between bank accounts, and prints a summary.
// ../README.md says what it prints, and how fast it has to be.
import kotlin.system.exitProcess

const val START = 1000L // what each account starts with

/**
 * The Park-Miller generator, which every language's version uses, so that they all make up the same
 * transfers from the same seed.
 */
class Random(private var state: Long) {
    fun next(): Long {
        state = state * 48271 % 2147483647
        return state
    }
}

class Account(val name: String, var balance: Long)

fun accountName(i: Long) = "acct" + i.toString().padStart(6, '0')

fun total(accounts: List<Account>): Long = accounts.sumOf { it.balance }

fun main() {
    val (count, transfers, seed) = readln().trim().split(Regex("\\s+")).map { it.toLong() }
    val accounts = (0 until count).map { Account(accountName(it), START) }
    // Finding an account by its name was the slowest part: a map finds it at once,
    // where looking through the list took time in proportion to the accounts.
    val byName = accounts.associateBy { it.name }

    val random = Random(seed)
    var refused = 0
    repeat(transfers.toInt()) {
        val source = byName.getValue(accountName(random.next() % count))
        val target = byName.getValue(accountName(random.next() % count))
        val amount = random.next() % 500 + 1
        if (source.balance < amount) {
            refused++
        } else {
            source.balance -= amount
            target.balance += amount
        }
    }
    // Adding up every account after every transfer was the other slow part. A transfer
    // moves money and can't change the total, so checking it once, at the end, catches
    // the same bugs.
    if (total(accounts) != count * START) {
        System.err.println("error: the total has changed")
        exitProcess(1)
    }

    println("transfers: $transfers, refused: $refused")
    println("total: ${total(accounts)}")
    val richest = accounts.sortedWith(compareBy({ -it.balance }, { it.name }))
    for (account in richest.take(5)) println("${account.name} ${account.balance}")
}
