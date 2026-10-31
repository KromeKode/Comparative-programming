// Clockwise order, so turning right adds 1 and turning left subtracts 1.
enum class Direction(val dx: Int, val dy: Int) {
    NORTH(0, 1),
    EAST(1, 0),
    SOUTH(0, -1),
    WEST(-1, 0);

    // mod, unlike %, never gives a negative result for a positive right side.
    fun turn(steps: Int) = entries[(ordinal + steps).mod(entries.size)]
}

class Robot {
    var x = 0
    var y = 0
    var facing = Direction.NORTH

    fun run(command: Char) {
        when (command) {
            'L' -> facing = facing.turn(-1)
            'R' -> facing = facing.turn(1)
            'F' -> {
                x += facing.dx
                y += facing.dy
            }
        }
    }
}

fun main() {
    while (true) {
        val line = readlnOrNull() ?: break
        if (line.isEmpty()) continue
        val bad = line.indexOfFirst { it !in "LRF" }
        if (bad != -1) {
            println("error: unknown command '${line[bad]}' at position ${bad + 1}")
            continue
        }
        val robot = Robot()
        line.forEach(robot::run)
        println("${robot.x} ${robot.y} ${robot.facing.name.lowercase()}")
    }
}
