// when is an expression: it gives a value. Branches never fall through.
fun describe(code: Int): String =
    when (code) {
        200 -> "ok"
        301, 302, 307, 308 -> "redirect"
        // Branches are tried from top to bottom, so 404 must come before the range.
        404 -> "not found"
        in 400..499 -> "client error" // .. includes both ends
        in 500..599 -> "server error"
        else -> "unknown"
    }

fun main() {
    for (code in listOf(200, 301, 308, 404, 418, 500, 503, 99)) {
        println("$code: ${describe(code)}")
    }
}
