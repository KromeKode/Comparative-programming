import kotlin.test.Test
import kotlin.test.assertEquals
import kotlin.test.assertFailsWith
import org.junit.jupiter.params.ParameterizedTest
import org.junit.jupiter.params.provider.CsvSource
import org.junit.jupiter.params.provider.ValueSource

class DurationTest {
    @ParameterizedTest
    @CsvSource(
        "45s, 45",
        "1h30m, 5400",
        "2d, 172800",
        "1d2h3m4s, 93784",
        // A number can be more than the next unit up holds, and it can be 0.
        "90m, 5400",
        "0s, 0",
        "1h0m, 3600",
    )
    fun parses(text: String, seconds: Long) {
        assertEquals(seconds, parseDuration(text))
    }

    @ParameterizedTest
    @ValueSource(
        strings =
            [
                "", // no parts
                "90", // a number with no unit
                "1h30", // the same, after a part
                "h", // a unit with no number
                "1x", // not a unit
                "1h1h", // a unit twice
                "30m1h", // units out of order
                "1h 30m", // anything else
                "-1h",
            ]
    )
    fun rejects(text: String) {
        assertFailsWith<IllegalArgumentException> { parseDuration(text) }
    }

    @ParameterizedTest
    @CsvSource(
        "5400, 1h30m",
        "93784, 1d2h3m4s",
        "59, 59s",
        // Units that are 0 are left out, wherever they are, unless everything is.
        "3600, 1h",
        "86401, 1d1s",
        "0, 0s",
    )
    fun formats(seconds: Long, text: String) {
        assertEquals(text, formatDuration(seconds))
    }

    @Test
    fun roundTrip() {
        for (seconds in 0L..<100_000L) {
            assertEquals(seconds, parseDuration(formatDuration(seconds)))
        }
    }
}
