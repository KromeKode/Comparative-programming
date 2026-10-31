interface ItemBag {
    fun add(item: String)

    fun addAll(items: List<String>)

    val size: Int
}

// A bag of items. addAll is written in terms of add. Kotlin classes and
// methods are final unless marked open, so Bag has to opt in to being extended.
open class Bag : ItemBag {
    private val items = mutableListOf<String>()

    override fun add(item: String) {
        items.add(item)
    }

    override fun addAll(items: List<String>) {
        for (item in items) {
            add(item) // calls the subclass's add, if it has one
        }
    }

    override val size
        get() = items.size
}

// Inheritance: a CountingBag is a Bag that also counts what was added.
class InheritedCountingBag : Bag() {
    var count = 0

    override fun add(item: String) {
        count++
        super.add(item)
    }

    override fun addAll(items: List<String>) {
        count += items.size
        super.addAll(items) // which calls add, which counts again
    }
}

// Composition: a CountingBag has a Bag. "by bag" writes out every ItemBag
// member as a call to bag, so only the members that change are written here.
class ComposedCountingBag(private val bag: Bag = Bag()) : ItemBag by bag {
    var count = 0

    override fun add(item: String) {
        count++
        bag.add(item)
    }

    override fun addAll(items: List<String>) {
        count += items.size
        bag.addAll(items) // the inner Bag calls its own add
    }
}

fun report(label: String, size: Int, count: Int) {
    println("$label: the bag holds $size items, the count says $count")
}

fun main() {
    val fruit = listOf("apple", "pear", "plum")

    val inherited = InheritedCountingBag()
    inherited.addAll(fruit)
    inherited.add("fig")
    report("inheritance", inherited.size, inherited.count)

    val composed = ComposedCountingBag()
    composed.addAll(fruit)
    composed.add("fig")
    report("composition", composed.size, composed.count)
}
