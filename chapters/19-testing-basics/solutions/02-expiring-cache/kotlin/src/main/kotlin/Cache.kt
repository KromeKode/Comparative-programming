/** Something that tells the time, in whole seconds. */
fun interface Clock {
    fun now(): Long
}

/** The computer's clock, which counts the seconds since 1970. */
object SystemClock : Clock {
    override fun now(): Long = System.currentTimeMillis() / 1000
}

/** Keeps each value for [ttl] seconds after it was put, and then forgets it. */
class Cache(private val ttl: Long, private val clock: Clock) {
    private class Entry(val value: String, val expires: Long) {
        fun isLive(now: Long) = now < expires
    }

    private val entries = mutableMapOf<String, Entry>()

    /** Stores [value] under [key] for the next [ttl] seconds, replacing any value there. */
    fun put(key: String, value: String) {
        val now = clock.now()
        entries.values.removeAll { !it.isLive(now) }
        entries[key] = Entry(value, now + ttl)
    }

    /** The value under [key], or null if there is none or it has expired. */
    fun get(key: String): String? {
        val entry = entries[key] ?: return null
        return if (entry.isLive(clock.now())) entry.value else null
    }

    /** How many entries haven't expired. */
    val size: Int
        get() {
            val now = clock.now()
            return entries.values.count { it.isLive(now) }
        }
}
