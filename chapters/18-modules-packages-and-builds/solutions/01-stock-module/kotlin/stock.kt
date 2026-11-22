// The stock package: how many of each item there are.
package stock

class Stock {
    // private: only code inside Stock can use counts. A sorted map keeps its keys
    // in order, so items() comes out sorted.
    private val counts = sortedMapOf<String, Long>()

    fun add(item: String, amount: Long) {
        counts[item] = count(item) + amount
    }

    /** Takes [amount] of [item] away, and returns true, if there are that many. */
    fun take(item: String, amount: Long): Boolean {
        if (count(item) < amount) return false
        counts[item] = count(item) - amount
        return true
    }

    /** How many of [item] there are: 0 for an item never added. */
    fun count(item: String): Long = counts[item] ?: 0

    /** The items there are any of, with their counts, in alphabetical order. */
    fun items(): List<Pair<String, Long>> =
        counts.filterValues { it > 0 }.map { (name, count) -> name to count }
}
