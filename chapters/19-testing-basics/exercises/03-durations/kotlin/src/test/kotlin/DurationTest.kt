// Write tests for parseDuration and formatDuration here, as @Test functions, or as
// @ParameterizedTest ones, which build.gradle.kts has what they need for. One is here
// to start with, and the README lists what they promise.
import kotlin.test.Test
import kotlin.test.assertEquals

class DurationTest {
    @Test
    fun parses() {
        assertEquals(5400, parseDuration("1h30m"))
    }
}
