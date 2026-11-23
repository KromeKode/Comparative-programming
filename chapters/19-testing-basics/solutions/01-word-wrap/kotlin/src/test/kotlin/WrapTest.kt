import kotlin.test.Test
import kotlin.test.assertEquals

class WrapTest {
    // Lines as long as they can be, and a last line that is shorter.
    @Test
    fun fillsLines() {
        assertEquals(
            listOf("the quick", "brown fox", "jumps"),
            wrap("the quick brown fox jumps", 10),
        )
    }

    // A line can be exactly the width, and no longer: "ab cd" is 5.
    @Test
    fun exactWidth() {
        assertEquals(listOf("the quick", "brown"), wrap("the quick brown", 9))
        assertEquals(listOf("ab", "cd"), wrap("ab cd", 4))
        assertEquals(listOf("ab cd"), wrap("ab cd", 5))
    }

    // One space between words, whatever the text had.
    @Test
    fun spaces() {
        assertEquals(listOf("the quick"), wrap("  the   quick  ", 20))
    }

    // No words, no lines.
    @Test
    fun noWords() {
        assertEquals(emptyList(), wrap("", 10))
        assertEquals(emptyList(), wrap("   ", 10))
    }

    // A word longer than the width gets a line of its own, wherever it is.
    @Test
    fun longWords() {
        assertEquals(listOf("extraordinary"), wrap("extraordinary", 5))
        assertEquals(listOf("an", "extraordinary", "day"), wrap("an extraordinary day", 5))
        assertEquals(listOf("a", "b"), wrap("a b", 1))
    }
}
