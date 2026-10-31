const val OVERDRAFT_LIMIT = 100L
const val OVERDRAFT_FEE = 5L
const val SAVINGS_WITHDRAWALS = 2

class AccountError(message: String) : Exception(message)

// What every account has. An abstract class cannot be made directly, and each
// kind must override the abstract members. Kotlin methods are final unless
// marked open, so withdraw says that a kind may replace it.
abstract class Account(val name: String) {
    var balance = 0L
        protected set

    fun deposit(amount: Long) {
        balance += amount
    }

    open fun withdraw(amount: Long) {
        checkWithdrawal(amount)
        balance -= amount
    }

    protected abstract fun checkWithdrawal(amount: Long)

    abstract fun monthEnd(): String
}

class Checking(name: String) : Account(name) {
    override fun checkWithdrawal(amount: Long) {
        if (balance - amount < -OVERDRAFT_LIMIT) {
            throw AccountError("over the overdraft limit of $OVERDRAFT_LIMIT")
        }
    }

    override fun monthEnd(): String {
        if (balance < 0) {
            balance -= OVERDRAFT_FEE
            return "fee $OVERDRAFT_FEE"
        }
        return "no fee"
    }
}

class Savings(name: String) : Account(name) {
    private var withdrawals = 0

    override fun checkWithdrawal(amount: Long) {
        if (withdrawals == SAVINGS_WITHDRAWALS) {
            throw AccountError("only $SAVINGS_WITHDRAWALS withdrawals a month")
        }
        if (amount > balance) {
            throw AccountError("insufficient funds")
        }
    }

    override fun withdraw(amount: Long) {
        super.withdraw(amount)
        withdrawals++
    }

    override fun monthEnd(): String {
        val interest = balance / 100
        balance += interest
        withdrawals = 0
        return "interest $interest"
    }
}

// A constructor can be passed around as a function with "::".
val kinds: Map<String, (String) -> Account> =
    mapOf("checking" to ::Checking, "savings" to ::Savings)

// mutableMapOf keeps its keys in the order they were added.
class Bank {
    private val accounts = mutableMapOf<String, Account>()

    private fun find(name: String) = accounts[name] ?: throw AccountError("no account named $name")

    fun run(words: List<String>) {
        when {
            words.size == 3 && words[0] == "open" -> {
                val (_, kind, name) = words
                val make = kinds[kind] ?: throw AccountError("unknown account kind: $kind")
                if (name in accounts) {
                    throw AccountError("$name already has an account")
                }
                accounts[name] = make(name)
                println("$name: opened a $kind account")
            }
            words.size == 3 && words[0] == "deposit" -> {
                val account = find(words[1])
                val amount = words[2].toLong()
                account.deposit(amount)
                println("${account.name}: deposited $amount, balance ${account.balance}")
            }
            words.size == 3 && words[0] == "withdraw" -> {
                val account = find(words[1])
                val amount = words[2].toLong()
                try {
                    account.withdraw(amount)
                } catch (error: AccountError) {
                    throw AccountError("${account.name}: ${error.message}")
                }
                println("${account.name}: withdrew $amount, balance ${account.balance}")
            }
            words == listOf("month") -> {
                for (account in accounts.values) {
                    val change = account.monthEnd()
                    println("${account.name}: $change, balance ${account.balance}")
                }
            }
        }
    }
}

fun main() {
    val bank = Bank()
    for (line in generateSequence(::readlnOrNull)) {
        val words = line.split(Regex("\\s+")).filter { it.isNotEmpty() }
        try {
            bank.run(words)
        } catch (error: AccountError) {
            println("error: ${error.message}")
        }
    }
}
