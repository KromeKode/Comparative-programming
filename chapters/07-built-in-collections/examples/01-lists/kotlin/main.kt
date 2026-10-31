fun main() {
    // listOf makes a read-only List; mutableListOf makes one you can change.
    val numbers = mutableListOf(3, 1, 4, 1, 5)
    println("numbers: $numbers") // a List prints as [3, 1, 4, 1, 5]
    println("length: ${numbers.size}")
    println("first: ${numbers.first()}, last: ${numbers.last()}")

    numbers.add(9)
    println("append 9: $numbers")
    numbers.add(1, 2) // add(index, value) inserts
    println("insert 2 at index 1: $numbers")
    val removed = numbers.removeAt(0)
    println("remove index 0 (was $removed): $numbers")

    println("contains 4: ${if (4 in numbers) "yes" else "no"}")
    println("index of 1: ${numbers.indexOf(1)}") // -1 if it is missing

    numbers.sort()
    println("sorted: $numbers")
    println("slice 1..4: ${numbers.subList(1, 4)}") // a view of the same list, no copy

    numbers.reverse()
    println("reversed: $numbers")
}
