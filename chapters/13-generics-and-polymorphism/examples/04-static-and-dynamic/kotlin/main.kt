interface Animal {
    val name: String

    fun sound(): String
}

class Cat(override val name: String) : Animal {
    override fun sound() = "meow"

    fun purr() = "prrr"
}

class Dog(override val name: String) : Animal {
    override fun sound() = "woof"
}

// On the JVM the type parameter is erased: this compiles to one method that
// takes a List, and each sound() call is looked up as it runs.
fun <A : Animal> chorus(animals: List<A>) =
    animals.joinToString(", ") { "${it.name} says ${it.sound()}" }

fun chorusAll(animals: List<Animal>) =
    animals.joinToString(", ") { "${it.name} says ${it.sound()}" }

// What the type parameter buys is the return type: the caller gets back a
// Cat, not just an Animal.
fun <A : Animal> longestName(animals: List<A>): A = animals.maxBy { it.name.length }

fun main() {
    val cats = listOf(Cat("Tom"), Cat("Kitty"))
    val dogs = listOf(Dog("Rex"))
    println("cats: ${chorus(cats)}")
    println("dogs: ${chorus(dogs)}")

    val everyone: List<Animal> = listOf(cats[0], dogs[0], cats[1])
    println("everyone: ${chorusAll(everyone)}")

    val cat = longestName(cats)
    println("longest name: ${cat.name}, who purrs ${cat.purr()}")
}
