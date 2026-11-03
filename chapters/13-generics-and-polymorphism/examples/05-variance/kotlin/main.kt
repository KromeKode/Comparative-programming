interface Animal {
    val name: String
}

class Cat(override val name: String) : Animal

class Dog(override val name: String) : Animal

// List is declared List<out E>: it only hands items out, so a List<Cat> can
// be used as a List<Animal>. Nothing can put a Dog into it that way.
fun names(animals: List<Animal>) = animals.joinToString(", ") { it.name }

fun main() {
    val cats = listOf(Cat("Tom"), Cat("Kitty"))
    println("names: ${names(cats)}")

    // A MutableList takes items in as well, so MutableList<Cat> is not a
    // MutableList<Animal>. A shelter that takes dogs needs its own list.
    val shelter: MutableList<Animal> = cats.toMutableList()
    shelter.add(Dog("Rex"))
    println("shelter: ${names(shelter)}")

    // A Comparator only takes items in, so one for any Animal can sort Cats:
    // sortedWith asks for a Comparator<in Cat>.
    val byName: Comparator<Animal> = compareBy { it.name }
    println("sorted by name: ${names(cats.sortedWith(byName))}")
}
