typealias Grid = List<CharArray>

// grid.toList() would copy only the outer list, and the copy would share its
// rows with the grid. Copy each row too.
fun Grid.deepCopy(): Grid = map { it.copyOf() }

// List(height) { row } would put one row in every slot, so make each row.
fun makeGrid(width: Int, height: Int, c: Char): Grid = List(height) { CharArray(width) { c } }

fun parseNumber(text: String): Int? {
    if (text.isEmpty() || !text.all { it in '0'..'9' }) {
        println("error: \"$text\" is not a number")
        return null
    }
    // toIntOrNull gives null for a number too big for an Int; that is
    // outside any grid, so use a value that is too.
    return text.toIntOrNull() ?: -1
}

fun oneCharacter(text: String): Char? {
    if (text.length != 1) {
        println("error: \"$text\" is not one character")
        return null
    }
    return text[0]
}

fun takes(command: String, want: Int, got: Int): Boolean {
    if (got != want) {
        val plural = if (want == 1) "" else "s"
        println("error: $command takes $want value$plural, got $got")
    }
    return got == want
}

fun fields(line: String) = line.split(Regex("\\s+")).filter { it.isNotEmpty() }

fun main() {
    val first = readlnOrNull() ?: return
    val (width, height) = fields(first).map { it.toInt() }
    var grid = makeGrid(width, height, '.')
    var saved: Grid? = null

    while (true) {
        val line = readlnOrNull() ?: break
        val parts = fields(line)
        if (parts.isEmpty()) continue
        val command = parts[0]
        val values = parts.drop(1)
        when (command) {
            "paint" -> {
                if (!takes(command, 3, values.size)) continue
                val x = parseNumber(values[0]) ?: continue
                val y = parseNumber(values[1]) ?: continue
                if (x !in 1..width || y !in 1..height) {
                    val where = "(${values[0]}, ${values[1]})"
                    println("error: $where is outside the $width by $height grid")
                    continue
                }
                val c = oneCharacter(values[2]) ?: continue
                grid[y - 1][x - 1] = c
            }
            "fill" -> {
                if (!takes(command, 1, values.size)) continue
                val c = oneCharacter(values[0]) ?: continue
                grid = makeGrid(width, height, c)
            }
            "save" -> if (takes(command, 0, values.size)) saved = grid.deepCopy()
            "restore" -> {
                if (!takes(command, 0, values.size)) continue
                if (saved == null) {
                    println("error: nothing saved")
                } else {
                    // grid = saved would share one grid, and the next paint
                    // would change the snapshot as well.
                    grid = saved.deepCopy()
                }
            }
            "show" ->
                if (takes(command, 0, values.size)) {
                    grid.forEach { println(String(it)) }
                    println()
                }
            else -> println("error: unknown command \"$command\"")
        }
    }
}
