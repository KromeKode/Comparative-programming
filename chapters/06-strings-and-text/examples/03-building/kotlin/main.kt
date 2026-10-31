import java.util.Locale

fun main() {
    val fruits = listOf("apple", "banana", "cherry")
    println("joined: ${fruits.joinToString(", ")}")

    // Strings are immutable. buildString gives the block a StringBuilder to append to.
    val built = buildString {
        for (n in 1..5) {
            if (n > 1) append('-')
            append(n)
        }
    }
    println("built: $built")

    println("padded: [${"Ada".padEnd(8)}] [${"90".padStart(8)}]")
    println("zero-padded: ${"42".padStart(5, '0')}")
    println("two decimals: ${String.format(Locale.ROOT, "%.2f", 3.14159)}")
}
