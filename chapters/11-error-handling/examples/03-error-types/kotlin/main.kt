// A sealed class: every subclass is in this file, so a when over a
// WithdrawException can be checked for missing cases.
sealed class WithdrawException(message: String) : Exception(message)

class InvalidAmount(val amount: Int) : WithdrawException("invalid amount: $amount")

class InsufficientFunds(val needed: Int, val available: Int) :
    WithdrawException("insufficient funds: need $needed, have $available")

class AccountFrozen : WithdrawException("account is frozen")

class Account(var balance: Int) {
    var frozen = false

    fun withdraw(amount: Int) {
        if (amount <= 0) throw InvalidAmount(amount)
        if (frozen) throw AccountFrozen()
        if (amount > balance) throw InsufficientFunds(amount, balance)
        balance -= amount
    }
}

fun attempt(account: Account, amount: Int) {
    try {
        account.withdraw(amount)
        println("withdraw $amount: ok, ${account.balance} left")
    } catch (e: WithdrawException) {
        println("withdraw $amount: error: ${e.message}")
        // Leave out a case and the compiler reports it.
        when (e) {
            is InsufficientFunds -> {
                account.withdraw(e.available)
                println("  took ${e.available} instead, ${account.balance} left")
            }
            is AccountFrozen -> println("  call the bank to unfreeze it")
            is InvalidAmount -> {}
        }
    }
}

fun main() {
    val account = Account(100)
    println("start with ${account.balance}")
    for (amount in listOf(30, -5, 100)) {
        attempt(account, amount)
    }
    println("freeze the account")
    account.frozen = true
    attempt(account, 10)
}
