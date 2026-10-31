// A sealed interface can only be implemented in this package and module, so the
// compiler knows every variant.
sealed interface Payment

data object Cash : Payment

data class Card(val last4: String) : Payment

data class Voucher(val code: String, val cents: Int) : Payment

fun money(cents: Int) = "%d.%02d".format(cents / 100, cents % 100)

// when on a sealed type must cover every variant. The `if` is a guard (Kotlin 2.2).
fun describe(payment: Payment, total: Int): String =
    when (payment) {
        Cash -> "cash: pay ${money(total)} in cash"
        is Card -> "card ending ${payment.last4}: charge ${money(total)} to the card"
        is Voucher if payment.cents >= total ->
            "voucher ${payment.code} (${money(payment.cents)}): covers the order"
        is Voucher -> {
            val left = money(total - payment.cents)
            "voucher ${payment.code} (${money(payment.cents)}): $left left to pay"
        }
    }

fun main() {
    val total = 1250
    val payments = listOf(Cash, Card("4242"), Voucher("SPRING", 500), Voucher("BIG", 2000))
    println("order total: ${money(total)}")
    for (payment in payments) {
        println(describe(payment, total))
    }
}
