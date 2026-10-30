fun main() {
    // Int / Int truncates toward zero, as in C and Java.
    println("7 / 2 = ${7 / 2}")
    println("-7 / 2 = ${-7 / 2}")
    // % is the remainder, which takes the sign of the left operand.
    println("7 % 3 = ${7 % 3}")
    println("-7 % 3 = ${-7 % 3}")
    println("7 % -3 = ${7 % -3}")
    // A Double on either side makes the result a Double.
    println("7.0 / 2 = ${7.0 / 2}")
    // floorDiv and mod round down instead, like Python's // and %.
    println("(-7).floorDiv(2) = ${(-7).floorDiv(2)}")
    println("(-7).mod(3) = ${(-7).mod(3)}")
}
