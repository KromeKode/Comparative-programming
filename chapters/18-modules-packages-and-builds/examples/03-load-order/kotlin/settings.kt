package settings

// Top-level properties of a file are set when the file's class is first used,
// the first time anything in the file is read or called.
val greeting = load()

private fun load(): String {
    println("settings: loading")
    return "Hello"
}
