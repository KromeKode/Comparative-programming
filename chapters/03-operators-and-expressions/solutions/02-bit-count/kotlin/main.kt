fun main() {
    while (true) {
        val line = readlnOrNull() ?: break
        val text = line.trim()
        if (text.isEmpty()) continue
        val n = text.toUInt() // UInt holds 0 to 4,294,967,295
        val width = UInt.SIZE_BITS - n.countLeadingZeroBits()
        println("$n: ${n.toString(2)} (ones ${n.countOneBits()}, width $width)")
    }
}
