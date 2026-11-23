// Write tests for wrap here, as @Test functions. One is here to start with, and the
// README lists what wrap promises.
import kotlin.test.Test
import kotlin.test.assertEquals

class WrapTest {
    @Test
    fun oneLine() {
        assertEquals(listOf("one two"), wrap("one two", 10))
    }
}
