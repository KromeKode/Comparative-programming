import kotlin.math.abs

fun main() {
    val sum = 0.1 + 0.2

    println("0.1 + 0.2 = $sum")
    println("equal to 0.3? ${if (sum == 0.3) "yes" else "no"}")
    println("close to 0.3? ${if (abs(sum - 0.3) < 1e-9) "yes" else "no"}")
}
