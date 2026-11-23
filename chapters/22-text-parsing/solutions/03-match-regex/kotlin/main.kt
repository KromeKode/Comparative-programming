// Prints each line of stdin that has a match for the pattern, as grep does, with a
// matcher of its own: the pattern becomes a tree, the tree becomes a program for a
// Thompson NFA, and the NFA reads each line once, keeping the set of states it could be
// in. So a line takes time in proportion to its length times the pattern's, whatever
// the pattern is.
import kotlin.system.exitProcess

/** What a "\" can make a literal: the characters that mean something, inside brackets or out. */
const val SPECIAL = "\\.[]()*+?|^$-"

class PatternError(val at: Int, message: String) : Exception(message) // at indexes code points

/**
 * The tree. A character, a ".", and brackets are each a set of characters: "a" is the range a to a,
 * and "." is the empty set, negated.
 */
sealed interface Node

data class Chars(val ranges: List<IntRange>, val negated: Boolean = false) : Node // code points

data class Anchor(val kind: Char) : Node // '^' or '$'

data class Concat(val items: List<Node>) : Node

data class Alternation(val options: List<Node>) : Node

data class Repeat(val item: Node, val kind: Char) : Node // '*', '+', or '?'

const val END = -1 // stands for the end of the pattern

/** Reads alternation = concat ("|" concat)*, concat = repeat*, and repeat = atom [*+?]. */
class Parser(pattern: String) {
    // A String is UTF-16, so these are its code points, and an index counts them, as
    // columns do.
    private val pattern: IntArray = pattern.codePoints().toArray()
    private var i = 0

    private fun peek() = if (i < pattern.size) pattern[i] else END

    private fun at(c: Char) = peek() == c.code

    fun parse(): Node {
        val node = alternation()
        // alternation() stops early only at a ")".
        if (i < pattern.size) throw PatternError(i, "unmatched \")\"")
        return node
    }

    private fun alternation(): Node {
        val options = mutableListOf(concat())
        while (at('|')) {
            i++
            options.add(concat())
        }
        return options.singleOrNull() ?: Alternation(options)
    }

    private fun concat(): Node {
        val items = mutableListOf<Node>()
        while (peek() != END && !at('|') && !at(')')) items.add(repeat())
        return Concat(items)
    }

    private fun atRepeat() = at('*') || at('+') || at('?')

    private fun repeat(): Node {
        // At the start, or after "(", "|", "^", "$", or another "*", "+", or "?".
        if (atRepeat()) throw PatternError(i, "nothing to repeat")
        if (at('^') || at('$')) {
            i++
            return Anchor(pattern[i - 1].toChar())
        }
        val item = atom()
        if (atRepeat()) {
            i++
            return Repeat(item, pattern[i - 1].toChar())
        }
        return item
    }

    private fun atom(): Node {
        when {
            at('(') -> {
                val start = i
                i++
                val node = alternation()
                if (!at(')')) throw PatternError(start, "unclosed \"(\"")
                i++
                return node
            }
            at('[') -> return brackets()
            at('.') -> {
                i++
                return Chars(emptyList(), negated = true)
            }
        }
        val c = char()
        return Chars(listOf(c..c))
    }

    /** A character, or a "\" and the character it makes a literal, as a code point. */
    private fun char(): Int {
        val c = pattern[i]
        if (c != '\\'.code) {
            i++
            return c
        }
        if (i + 1 == pattern.size) throw PatternError(i, "trailing backslash")
        val escaped = pattern[i + 1]
        if (SPECIAL.none { it.code == escaped }) throw PatternError(i, "invalid escape")
        i += 2
        return escaped
    }

    private fun brackets(): Node {
        val start = i
        i++ // the [
        val negated = at('^')
        if (negated) i++
        if (at(']')) throw PatternError(start, "empty brackets")
        val ranges = mutableListOf<IntRange>()
        while (!at(']')) {
            if (peek() == END) throw PatternError(start, "unclosed \"[\"")
            val from = i
            val low = char()
            var high = low
            // A "-" makes a range, unless it's last: "[a-]" is "a" and "-".
            if (at('-') && i + 1 < pattern.size && pattern[i + 1] != ']'.code) {
                i++
                high = char()
                if (high < low) throw PatternError(from, "range out of order")
            }
            ranges.add(low..high)
        }
        i++ // the ]
        return Chars(ranges, negated)
    }
}

/**
 * The program. ReadChars reads a character, if it's in the set, and goes on to the next
 * instruction; Split goes on to both x and y; Jump goes on to x; LineStart and LineEnd go on if the
 * position is the start or the end of the line; and Match is a match.
 */
sealed interface Instruction {
    fun accepts(c: Int) = false
}

class ReadChars(val ranges: List<IntRange>, val negated: Boolean) : Instruction {
    override fun accepts(c: Int) = ranges.any { c in it } != negated
}

class Split(val x: Int, var y: Int = 0) : Instruction

class Jump(var x: Int = 0) : Instruction

data object LineStart : Instruction

data object LineEnd : Instruction

data object Match : Instruction

fun compile(node: Node, program: MutableList<Instruction>) {
    val start = program.size
    when (node) {
        is Chars -> program.add(ReadChars(node.ranges, node.negated))
        is Anchor -> program.add(if (node.kind == '^') LineStart else LineEnd)
        is Concat -> node.items.forEach { compile(it, program) }
        is Alternation -> {
            // split L1, L2; L1: first; jump END; L2: split L3, L4; L3: second; ...
            val jumps = mutableListOf<Jump>()
            for (option in node.options.dropLast(1)) {
                val split = Split(program.size + 1)
                program.add(split)
                compile(option, program)
                jumps.add(Jump())
                program.add(jumps.last())
                split.y = program.size
            }
            compile(node.options.last(), program)
            jumps.forEach { it.x = program.size }
        }
        is Repeat ->
            when (node.kind) {
                '*' -> {
                    // L1: split L2, L3; L2: item; jump L1; L3:
                    val split = Split(start + 1)
                    program.add(split)
                    compile(node.item, program)
                    program.add(Jump(start))
                    split.y = program.size
                }
                '+' -> {
                    // L1: item; split L1, L2; L2:
                    compile(node.item, program)
                    program.add(Split(start, program.size + 1))
                }
                else -> {
                    // split L1, L2; L1: item; L2:
                    val split = Split(start + 1)
                    program.add(split)
                    compile(node.item, program)
                    split.y = program.size
                }
            }
    }
}

fun compilePattern(pattern: String): List<Instruction> {
    val program = mutableListOf<Instruction>()
    compile(Parser(pattern).parse(), program)
    program.add(Match)
    return program
}

/**
 * The ReadChars instructions that pcs lead to without reading a character, at the position at, and
 * whether one of them leads to Match.
 */
fun follow(
    program: List<Instruction>,
    pcs: List<Int>,
    at: Int,
    length: Int,
): Pair<List<Int>, Boolean> {
    val waiting = mutableListOf<Int>()
    val seen = BooleanArray(program.size)
    val stack = ArrayDeque(pcs)
    while (stack.isNotEmpty()) {
        val pc = stack.removeLast()
        if (seen[pc]) continue // so that a loop such as (a*)* doesn't go around forever
        seen[pc] = true
        when (val instruction = program[pc]) {
            is ReadChars -> waiting.add(pc)
            is Split -> stack.addAll(listOf(instruction.y, instruction.x))
            is Jump -> stack.addLast(instruction.x)
            LineStart -> if (at == 0) stack.addLast(pc + 1)
            LineEnd -> if (at == length) stack.addLast(pc + 1)
            Match -> return Pair(emptyList(), true)
        }
    }
    return Pair(waiting, false)
}

/**
 * Whether a match starts anywhere in the line, of code points: instruction 0 starts at each
 * position.
 */
fun search(program: List<Instruction>, line: IntArray): Boolean {
    var waiting = listOf<Int>()
    for (at in 0..line.size) {
        val (reached, matched) = follow(program, waiting + 0, at, line.size)
        if (matched) return true
        waiting =
            if (at == line.size) emptyList()
            else reached.filter { program[it].accepts(line[at]) }.map { it + 1 }
    }
    return false
}

fun main(args: Array<String>) {
    if (args.size != 1) {
        System.err.println("usage: rematch PATTERN")
        exitProcess(2)
    }
    val program =
        try {
            compilePattern(args[0])
        } catch (err: PatternError) {
            System.err.println("rematch: column ${err.at + 1}: ${err.message}")
            exitProcess(2)
        }
    val text = System.`in`.readBytes().decodeToString()
    // Without what comes after the last "\n", or the empty input.
    val lines = text.split("\n").let { if (it.last() == "") it.dropLast(1) else it }
    val found = lines.filter { search(program, it.codePoints().toArray()) }
    print(found.joinToString("") { "$it\n" })
    if (found.isEmpty()) exitProcess(1)
}
