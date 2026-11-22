import kotlin.test.Test
import kotlin.test.assertEquals
import kotlin.test.assertFailsWith
import org.junit.jupiter.params.ParameterizedTest
import org.junit.jupiter.params.provider.CsvSource

class RomanTest {
    // JUnit runs this once for each line, and names each run after its values, so
    // a failure says which case it was.
    @ParameterizedTest
    @CsvSource(
        "1, I",
        "4, IV",
        "9, IX",
        "14, XIV",
        "40, XL",
        "90, XC",
        "400, CD",
        "1994, MCMXCIV",
        "2024, MMXXIV",
        "3999, MMMCMXCIX",
    )
    fun numerals(n: Int, want: String) {
        assertEquals(want, toRoman(n))
    }

    // The errors are part of what toRoman promises, so they get tests too.
    @Test
    fun outOfRange() {
        for (n in listOf(0, 4000, -1)) {
            assertFailsWith<IllegalArgumentException> { toRoman(n) }
        }
    }
}
