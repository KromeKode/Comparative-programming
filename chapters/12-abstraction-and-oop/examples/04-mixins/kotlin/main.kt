// An interface can give a method a default body. A class that implements
// Describe must supply name, and gets describe for free.
interface Describe {
    val name: String

    fun describe() = "this is the $name"
}

// An interface cannot store anything, so Logged declares the history it needs
// and each class supplies the list.
interface Logged {
    val name: String
    val history: MutableList<String>

    fun log(event: String) {
        history.add("$name: $event")
    }
}

class Door(override val name: String) : Describe, Logged {
    override val history = mutableListOf<String>()

    fun open() = log("opened")

    fun close() = log("closed")
}

class Light(override val name: String) : Describe, Logged {
    override val history = mutableListOf<String>()
    var on = false

    fun switchOn() {
        on = true
        log("switched on")
    }

    // Light replaces the default describe, and reuses it.
    override fun describe() = "${super.describe()}, and it is ${if (on) "on" else "off"}"
}

fun main() {
    val door = Door("front door")
    val light = Light("porch light")
    door.open()
    door.close()
    light.switchOn()

    for (thing in listOf(door, light)) {
        println(thing.describe())
        for (line in thing.history) {
            println("  $line")
        }
    }
}
