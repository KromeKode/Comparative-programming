class User(val name: String)

val numbers = listOf(10, 20, 30)
val users = mapOf(1 to User("ada"))

fun findUser(id: Int): User? = users[id]

// Run action and report what it returned or which exception it threw.
fun attempt(label: String, action: () -> Any?) {
    try {
        println("$label: ${action()}")
    } catch (e: Exception) {
        val message = e.message?.let { ": $it" } ?: ""
        println("$label: ${e::class.simpleName}$message")
    }
}

fun main() {
    val index = 5
    val zero = 0
    attempt("numbers[5]") { numbers[index] }
    attempt("10 / zero") { 10 / zero }
    attempt("findUser(7)!!.name") { findUser(7)!!.name }
    attempt("check(numbers.size == 4)") {
        check(numbers.size == 4) { "expected 4 numbers" }
        "passed"
    }
    // The checked version returns null instead.
    attempt("numbers.getOrNull(5)") { numbers.getOrNull(index) }
}
