// Converts temperatures, such as 21.5c, -40f, or 300k, to Celsius, Fahrenheit, or
// Kelvin. Clikt reads the options from the properties of the class Temp, and writes the
// help from them. It exits with status 1 when the command line is wrong, so main
// catches its errors to exit with 2, as the other languages' tools do.
import com.github.ajalt.clikt.core.CliktCommand
import com.github.ajalt.clikt.core.CliktError
import com.github.ajalt.clikt.core.Context
import com.github.ajalt.clikt.core.UsageError
import com.github.ajalt.clikt.core.parse
import com.github.ajalt.clikt.parameters.arguments.argument
import com.github.ajalt.clikt.parameters.arguments.multiple
import com.github.ajalt.clikt.parameters.options.default
import com.github.ajalt.clikt.parameters.options.option
import com.github.ajalt.clikt.parameters.types.choice
import com.github.ajalt.clikt.parameters.types.int
import com.github.ajalt.clikt.parameters.types.restrictTo
import java.util.Locale
import kotlin.system.exitProcess

fun toCelsius(t: Double, unit: Char): Double =
    when (unit) {
        'f' -> (t - 32) * 5 / 9
        'k' -> t - 273.15
        else -> t
    }

fun fromCelsius(t: Double, unit: Char): Double =
    when (unit) {
        'f' -> t * 9 / 5 + 32
        'k' -> t + 273.15
        else -> t
    }

data class Temperature(val text: String, val value: Double, val unit: Char)

// A number followed by c, f, or k, or null.
fun parseTemperature(text: String): Temperature? {
    val unit = text.lastOrNull()
    if (unit == null || unit !in "cfk") return null
    val value = text.dropLast(1).toDoubleOrNull()
    return if (value != null && value.isFinite()) Temperature(text, value, unit) else null
}

class Temp : CliktCommand(name = "temp") {
    val to by
        option("-t", "--to", metavar = "UNIT", help = "the unit to convert to")
            .choice("c", "f", "k")
            .default("c")
    val digits by
        option("-d", "--digits", metavar = "DIGITS", help = "digits after the point")
            .int()
            .restrictTo(0..9)
            .default(1)
    val texts by argument(name = "TEMPERATURE").multiple(required = true)

    override fun help(context: Context) = "Converts temperatures, such as 21.5c, -40f, or 300k."

    override fun run() {
        // Check every temperature before printing any, so that a mistake prints nothing.
        val temperatures = texts.map { text ->
            parseTemperature(text)
                ?: run {
                    System.err.println("temp: not a temperature: $text")
                    exitProcess(2)
                }
        }
        for ((text, value, unit) in temperatures) {
            val converted = fromCelsius(toCelsius(value, unit), to[0])
            // Locale.ROOT, or a German computer would print 21,5.
            println(String.format(Locale.ROOT, "%s = %.${digits}f%s", text, converted, to))
        }
    }
}

fun main(args: Array<String>) {
    val command = Temp()
    try {
        command.parse(args)
    } catch (error: CliktError) {
        // What Temp().main(args) would do, but with status 2 for a usage error.
        command.echoFormattedHelp(error)
        exitProcess(if (error is UsageError) 2 else error.statusCode)
    }
}
