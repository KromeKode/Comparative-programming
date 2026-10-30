const val READ = 0b001
const val WRITE = 0b010
const val EXEC = 0b100

fun bits8(n: Int) = n.toString(2).padStart(8, '0')

fun countOnes(n: Int): Int {
    var count = 0
    var rest = n
    while (rest != 0) {
        count += rest and 1
        rest = rest ushr 1
    }
    return count
}

fun main() {
    // Kotlin spells the bit operators as words: and, or, xor, inv, shl, shr, ushr.
    var perms = READ or WRITE
    println("read | write: ${bits8(perms)}")
    // Named infix calls bind tighter than !=, so no parentheses are needed.
    println("has write? ${if (perms and WRITE != 0) "yes" else "no"}")
    println("has exec? ${if (perms and EXEC != 0) "yes" else "no"}")
    perms = perms and WRITE.inv() // there is no &=, so assign the result
    println("clear write: ${bits8(perms)}")
    perms = perms xor EXEC
    println("toggle exec: ${bits8(perms)}")
    println("5 << 2 = ${5 shl 2}")
    println("-16 >> 2 = ${-16 shr 2}")
    println("set bits in 10110: ${countOnes("10110".toInt(2))}")
}
