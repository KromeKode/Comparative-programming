// Write tests for the cache here, as @Test functions. One is here to start with, on
// the computer's clock, which can't show what happens as time passes without the
// test waiting for it. The README lists what the cache promises.
import kotlin.test.Test
import kotlin.test.assertEquals

class CacheTest {
    @Test
    fun putThenGet() {
        val cache = Cache(60, SystemClock)
        cache.put("a", "1")
        assertEquals("1", cache.get("a"))
    }
}
