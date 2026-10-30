fun main() {
    val s = "Zo\u00eb \uD83D\uDC4D\uD83C\uDFFD"

    val bytes = s.toByteArray().joinToString(" ") { "%02X".format(it) }
    println("UTF-8 bytes: $bytes")

    // codePoints() walks code points; s[i] and s.length count UTF-16 code units.
    val codePoints = s.codePoints().toArray().joinToString(" ") { "U+%04X".format(it) }
    println("code points: $codePoints")

    // substring takes UTF-16 positions, so find where the third code point ends.
    println("first 3 code points: ${s.substring(0, s.offsetByCodePoints(0, 3))}")
}
