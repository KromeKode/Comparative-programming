// Gradle compiles src/test/kotlin separately from the program, and JUnit runs every
// method annotated @Test, which it finds by reflection.
import kotlin.test.Test
import kotlin.test.assertFalse
import kotlin.test.assertTrue

class LeapTest {
    @Test
    fun divisibleBy4() {
        assertTrue(isLeapYear(2024))
        assertFalse(isLeapYear(2023))
    }

    @Test
    fun centuries() {
        assertFalse(isLeapYear(1900))
        assertTrue(isLeapYear(2000))
    }
}
