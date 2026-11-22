/** Wraps [text] into lines of at most [width] characters, breaking only at spaces. */
fun wrap(text: String, width: Int): List<String> {
    val lines = mutableListOf<String>()
    var line = ""
    for (word in text.split(" ")) {
        if (word.isEmpty()) {
            continue
        }
        if (line.isEmpty()) {
            line = word
        } else if (line.length + 1 + word.length <= width) {
            line += " $word"
        } else {
            lines += line
            line = word
        }
    }
    if (line.isNotEmpty()) {
        lines += line
    }
    return lines
}
