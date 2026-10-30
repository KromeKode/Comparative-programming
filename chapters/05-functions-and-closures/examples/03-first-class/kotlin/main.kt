fun addOne(x: Int) = x + 1

fun triple(x: Int) = x * 3

// (Int) -> Int is the type of any function from Int to Int.
fun applyTwice(f: (Int) -> Int, x: Int) = f(f(x))

fun compose(outer: (Int) -> Int, inner: (Int) -> Int): (Int) -> Int = { x -> outer(inner(x)) }

fun main() {
    // ::addOne refers to the function without calling it.
    println("apply_twice(add_one, 5) = ${applyTwice(::addOne, 5)}")
    println("apply_twice(triple, 5) = ${applyTwice(::triple, 5)}")
    // A lambda with one parameter can call it `it`.
    println("apply_twice(square, 3) = ${applyTwice({ it * it }, 3)}")
    val addThenTriple = compose(::triple, ::addOne)
    println("compose(triple, add_one)(5) = ${addThenTriple(5)}")
}
