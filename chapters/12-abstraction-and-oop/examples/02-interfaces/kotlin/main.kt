data class Row(val name: String, val score: Int)

// An interface lists what an exporter must have. A class says ": Exporter"
// to implement it, and must override every member.
interface Exporter {
    val name: String

    fun export(rows: List<Row>): String
}

class CsvExporter(private val separator: String) : Exporter {
    override val name = "csv"

    override fun export(rows: List<Row>) =
        (listOf("name${separator}score") + rows.map { "${it.name}$separator${it.score}" })
            .joinToString("\n")
}

class JsonExporter : Exporter {
    override val name = "json"

    override fun export(rows: List<Row>) =
        rows.joinToString(",", "[", "]") { """{"name":"${it.name}","score":${it.score}}""" }
}

class MarkdownExporter : Exporter {
    override val name = "markdown"

    override fun export(rows: List<Row>) =
        (listOf("| name | score |", "| --- | --- |") + rows.map { "| ${it.name} | ${it.score} |" })
            .joinToString("\n")
}

// An object expression makes a one-off object that implements an interface.
val summaryExporter =
    object : Exporter {
        override val name = "summary"

        override fun export(rows: List<Row>): String {
            val best = rows.maxBy { it.score }
            return "${rows.size} rows, top score ${best.score} by ${best.name}"
        }
    }

fun run(exporter: Exporter, rows: List<Row>) {
    println("== ${exporter.name}")
    println(exporter.export(rows))
}

fun main() {
    val rows = listOf(Row("ada", 90), Row("grace", 85), Row("alan", 78))
    val exporters = listOf(CsvExporter(","), JsonExporter(), MarkdownExporter(), summaryExporter)
    for (exporter in exporters) {
        run(exporter, rows)
    }
}
