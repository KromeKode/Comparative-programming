fun main() {
    while (true) {
        val line = readlnOrNull() ?: break
        if (line.isBlank()) continue
        val x = line.trim().toInt()
        // toByte and toUByte keep the lowest 8 bits, like `as u8` in Rust.
        println("$x -> u8 ${x.toUByte()}, i8 ${x.toByte()}")
    }
}
