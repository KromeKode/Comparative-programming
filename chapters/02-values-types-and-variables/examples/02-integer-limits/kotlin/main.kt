fun main() {
    val i32Max = Int.MAX_VALUE
    println("int32 max: $i32Max")
    println("int32 max + 1: ${i32Max + 1}") // wraps silently, as Java does

    // Math.addExact throws instead of wrapping.
    val checked =
        try {
            Math.addExact(i32Max, 1).toString()
        } catch (e: ArithmeticException) {
            "ArithmeticException: ${e.message}"
        }
    println("Math.addExact(int32 max, 1): $checked")

    val i64Max = Long.MAX_VALUE
    println("int64 max: $i64Max")
    println("int64 max + 1: ${i64Max + 1}")
}
