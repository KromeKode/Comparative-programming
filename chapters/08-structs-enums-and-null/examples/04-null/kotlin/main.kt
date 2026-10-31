// String? may hold null; plain String never does, and the compiler checks it.
data class User(val name: String, val nickname: String? = null)

// find gives null when nothing matches.
fun find(users: List<User>, name: String): User? = users.find { it.name == name }

fun main() {
    val users = listOf(User("ada", "Countess"), User("alan"), User("grace", "Amazing Grace"))

    // a ?: b (the Elvis operator) is b only when a is null.
    for (user in users) {
        println("${user.name} is shown as ${user.nickname ?: user.name}")
    }

    // a?.b stops and gives null when a is null.
    val lengths = users.map { it.nickname?.length ?: 0 }
    println("nickname lengths: ${lengths.joinToString(" ")}")

    for (name in listOf("alan", "linus")) {
        println("find \"$name\": ${find(users, name)?.name ?: "nobody"}")
    }

    for (name in listOf("ada", "alan", "linus")) {
        println("nickname of \"$name\": ${find(users, name)?.nickname ?: "none"}")
    }
}
