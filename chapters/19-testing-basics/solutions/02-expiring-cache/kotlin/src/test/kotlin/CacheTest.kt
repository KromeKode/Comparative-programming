import kotlin.test.Test
import kotlin.test.assertEquals
import kotlin.test.assertNull

class CacheTest {
    // JUnit makes a new CacheTest for each test, so each starts with time 0 and an
    // empty cache. Clock is a fun interface, so a lambda can be one: this one says
    // whatever time the test sets.
    private var time = 0L
    private val cache = Cache(10) { time }

    @Test
    fun putThenGet() {
        cache.put("a", "1")
        assertEquals("1", cache.get("a"))
        assertNull(cache.get("b"))
        assertEquals(1, cache.size)
    }

    // An entry put at time t is there until t + ttl, and gone from then on.
    @Test
    fun expiresAfterTtl() {
        time = 100
        cache.put("a", "1")
        time = 109
        assertEquals("1", cache.get("a"))
        time = 110
        assertNull(cache.get("a"))
        assertEquals(0, cache.size)
    }

    // Getting an entry doesn't make it last longer.
    @Test
    fun getDoesNotRefresh() {
        cache.put("a", "1")
        time = 5
        assertEquals("1", cache.get("a"))
        time = 10
        assertNull(cache.get("a"))
    }

    // Putting a key again replaces its value, and its ttl starts again.
    @Test
    fun putReplaces() {
        cache.put("a", "1")
        time = 5
        cache.put("a", "2")
        assertEquals(1, cache.size)
        time = 14
        assertEquals("2", cache.get("a"))
        time = 15
        assertNull(cache.get("a"))
    }

    // Each key has its own time: one expiring, or a put of another, leaves the rest.
    @Test
    fun keysAreSeparate() {
        cache.put("a", "1")
        time = 5
        cache.put("b", "2")
        assertEquals("1", cache.get("a"))
        assertEquals("2", cache.get("b"))
        assertEquals(2, cache.size)
        time = 10
        assertNull(cache.get("a"))
        assertEquals("2", cache.get("b"))
        assertEquals(1, cache.size)
    }
}
